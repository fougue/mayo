/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_dxf.h"

#include "../base/brep_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/filepath.h"
#include "../base/geom_utils.h"
#include "../base/math_utils.h"
#include "../base/mesh_utils.h"
#include "../base/messenger.h"
#include "../base/occ_handle.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/string_conv.h"
#include "../base/tkernel_utils.h"
#include "../base/unit_system.h"
#include "aci_table.h"
#include "dxf.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRep_Builder.hxx>
#include <BSplCLib.hxx>
#include <Font_BRepTextBuilder.hxx>
#include <Font_FontMgr.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <Graphic3d_VerticalTextAlignment.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Resource_Unicode.hxx>
#include <TDataStd_Name.hxx>
#include <TopoDS_Edge.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Trsf.hxx>

#include <fmt/format.h>
#include <gsl/narrow>
#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

#include <variant>

//#define MAYO_IO_DXF_DEBUG_TRACE 1

namespace Mayo::IO {

namespace {

template<typename T>
using ConstRefWrap = std::reference_wrapper<const T>;

bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}

std::string toLowerCase_C(const std::string& str)
{
    std::string lstr = str;
    for (char& c : lstr)
        c = std::tolower(c, std::locale::classic());

    return lstr;
}

const Enumeration& systemFontNames()
{
    static Enumeration fontNames;
    static TColStd_SequenceOfHAsciiString seqFontName;
    if (fontNames.empty()) {
        OccHandle<Font_FontMgr> fontMgr = Font_FontMgr::GetInstance();
        fontMgr->GetAvailableFontsNames(seqFontName);
        int i = 0;
        for (const OccHandle<TCollection_HAsciiString>& fontName : seqFontName)
            fontNames.addItem(i++, { {}, to_stdStringView(fontName->String()) });
    }

    return fontNames;
}

gp_Pnt toOccPnt(const DxfCoords& coords)
{
    return { coords.x, coords.y, coords.z };
}

gp_Vec toOccVec(const DxfCoords& coords)
{
    return { coords.x, coords.y, coords.z };
}

gp_Dir toOccDir(const DxfCoords& coords, const gp_Dir& defaultDir = gp::DZ())
{
    if (!std::isfinite(coords.x) || !std::isfinite(coords.y) || !std::isfinite(coords.z))
        return defaultDir;

    const gp_Vec v = toOccVec(coords);
    if (v.SquareMagnitude() <= Precision::SquareConfusion())
        return defaultDir;

    return v;
}

struct Frame {
    gp_Dir u;
    gp_Dir v;
    gp_Dir w;
};

struct Placement {
    gp_Ax2 ax2;
    Frame frame;
};

Frame makeOcsFrame(const gp_Dir& w)
{
    // Choisir un a non colinéaire à w (45° ~ 0.70710678)
    gp_Dir a = (std::abs(w.Z()) < 0.7071067811865476) ? gp::DZ() : gp::DX();

    // Projection de a dans le plan orthogonal à w : uvec = a - (a·w) w
    gp_Vec uvec = gp_Vec(a) - gp_Vec(w) * (a.Dot(w));
    if (uvec.SquareMagnitude() <= Precision::SquareConfusion()) {
        // a ≈ colinéaire à w → essayer un autre a
        a = gp::DY();
        uvec = gp_Vec(a) - gp_Vec(w) * (a.Dot(w));
    }

    const gp_Dir u{uvec};                  // normalisation implicite de gp_Dir
    const gp_Dir v{gp_Vec(w) ^ gp_Vec(u)}; // repère droit : v = w × u
    return {u, v, w};
}

Placement makePlacementFromOcs(
        const DxfCoords& centerOcs, const DxfCoords& extrusionDir, const gp_Pnt& originWcs = gp::Origin()
    )
{
    Placement pl;
    pl.frame = makeOcsFrame(toOccDir(extrusionDir));
    // Mapping OCS -> WCS
    const gp_Pnt cw = originWcs.Translated(
        gp_Vec{pl.frame.u} * centerOcs.x
        + gp_Vec{pl.frame.v} * centerOcs.y
        + gp_Vec{pl.frame.w} * centerOcs.z
    );
    // Z=w, X=u → déterministic orientation
    pl.ax2 = gp_Ax2{cw, pl.frame.w, pl.frame.u};
    return pl;
}

// Normalize angle `a` in degrees within [0,360)
double normalizeAngleDeg(double a)
{
    double r = std::fmod(a, 360.);
    if (r < 0.0)
        r += 360.0;

    return r;
}

// Point on circle from OCS angle(radians)
// P(θ) = C + R*(cosθ * u + sinθ * v)
gp_Pnt pointOnCircle(const gp_Pnt& C, double R, double theta, const gp_Dir& u, const gp_Dir& v)
{
    return C.Translated(gp_Vec{u} * (R * std::cos(theta)) + gp_Vec{v} * (R * std::sin(theta)));
}

TopoDS_Shape makeExtrusionShape(const TopoDS_Shape& shape, double thickness, const gp_Dir& extrusionDir)
{
    if (std::abs(thickness) > Precision::Confusion()) {
        const gp_Vec h = gp_Vec{extrusionDir} * thickness;
        return BRepPrimAPI_MakePrism(shape, h).Shape();
    }
    else {
        return shape;
    }
}

// Tests if `scale' is uniform(isotropic)
bool isUniformScale(const DxfScale& scale)
{
    return MathUtils::fuzzyEqual(scale.x, scale.y) && MathUtils::fuzzyEqual(scale.y, scale.z);
}

// Is `scale` a unit-like triple{±1, ±1, ±1}?
bool isUnitScale(const DxfScale& scale)
{
    return MathUtils::fuzzyEqual(std::abs(scale.x), 1.)
           && MathUtils::fuzzyEqual(std::abs(scale.y), 1)
           && MathUtils::fuzzyEqual(std::abs(scale.z), 1)
        ;
}

gp_Trsf unitScaleTrsf(const gp_Pnt& pnt, const Frame& frame, const DxfScale& scale)
{
    gp_Trsf trsf;
    assert(isUnitScale(scale));

    const double sx = scale.x;
    const double sy = scale.y;
    const double sz = scale.z;
    const int negativeCount = (sx < .0 ? 1 : 0) + (sy < 0. ? 1 : 0) + (sz < 0. ? 1 : 0);
    if (negativeCount == 0) {
    }
    else if (negativeCount == 1) {
        // Mirror normal to the corresponding axis
        const gp_Dir n = (sx < 0) ? frame.u : (sy < 0) ? frame.v : frame.w;
        trsf.SetMirror(gp_Ax2(pnt, n));   // plane passing through `pnt` which normal `n`
    }
    else if (negativeCount == 2) {
        // Rotate by 180° around axis corresponding to positive sign
        const gp_Dir axis = (sx > 0) ? frame.u : (sy > 0) ? frame.v : frame.w;
        trsf.SetRotation(gp_Ax1(pnt, axis), MathConst::pi);
    }
    else {
        // Central symmetry
        trsf.SetMirror(pnt);
    }

    return trsf;
}

// OCS point -> WCS(with ⟨u,v,w⟩ frame)
gp_Pnt ocsPointToWcs(const DxfCoords& pnt, const Frame& frame)
{
    return gp::Origin().Translated(
        gp_Vec{frame.u}*pnt.x + gp_Vec{frame.v}*pnt.y + gp_Vec{frame.w}*pnt.z
    );
}

// OCS vector -> WCS(with ⟨u,v,w⟩ frame)
gp_Vec ocsVecToWcs(const DxfCoords& vec, const Frame& frame)
{
    return gp_Vec{frame.u}*vec.x + gp_Vec{frame.v}*vec.y + gp_Vec{frame.w}*vec.z;
}

} // namespace

class DxfReader::Internal : public CDxfRead {
public:
    bool read(const FilePath& filepath, TaskProgress* progress = nullptr);

    void setMessenger(Messenger* messenger) { m_messenger = messenger; }
    void setParameters(const DxfReader::Parameters& params) { m_params = params; }

    void ReportError(const std::string& msg) override;

    TopoDS_Shape createEntityShape(const Dxf_EntityVariant& entityVar);
    TopoDS_Shape createBlockShape(const Dxf_BLOCK& block);

    TopoDS_Shape createShape(const Dxf_3DFACE& face);
    TopoDS_Shape createShape(const Dxf_ARC& arc);
    TopoDS_Shape createShape(const Dxf_CIRCLE& circle);
    TopoDS_Shape createShape(const Dxf_ELLIPSE& ellipse);
    TopoDS_Shape createShape(const Dxf_INSERT& insert);
    TopoDS_Shape createShape(const Dxf_LINE& line);
    TopoDS_Shape createShape(const Dxf_LWPOLYLINE& polyline);
    TopoDS_Shape createShape(const Dxf_MTEXT& mtext);
    TopoDS_Shape createShape(const Dxf_POINT& point);
    TopoDS_Shape createShape(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShape(const Dxf_SOLID& solid);
    TopoDS_Shape createShape(const Dxf_SPLINE& spline);
    TopoDS_Shape createShape(const Dxf_TEXT& text);

    static TopoDS_Shape createSplineFromPolesAndKnots(const Dxf_SPLINE& spline);
    static TopoDS_Shape createInterpolationSpline(const Dxf_SPLINE& spline);

    gp_Pnt toPnt(const DxfCoords& coords) const;
    void addShape(const TopoDS_Shape& shape, const Dxf_BaseEntity& srcEntity);

    static TopoDS_Face makeFace(const Dxf_QuadBase& quad);

protected:
    void getLine() override;
    bool setSourceEncoding(const std::string& codepage) override;
    std::string toUtf8(const std::string& strSource) override;

private:
    TopoDS_Shape createShapePolygonMesh3d(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapePolyfaceMesh(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapePolyline3d(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapePolyline2d(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapeCurveFit(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapeSplineFit(const Dxf_POLYLINE& polyline);

    enum class BlockState {
        Unvisited, Visiting, Resolved
    };
    struct OccBlock {
        BlockState state = BlockState::Unvisited;
        TopoDS_Shape shape;
    };

    unsigned m_lineCounter = 0;
    Messenger* m_messenger = nullptr;
    DxfReader::Parameters m_params;
    TaskProgress* m_progress = nullptr;
    std::uintmax_t m_fileSize = 0;
    std::uintmax_t m_fileReadSize = 0;
    Resource_FormatType m_srcEncoding = Resource_ANSI;
    std::unordered_map<DxfStringRef, OccBlock> m_mapOccBlock;
};

class DxfReader::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::DxfReader::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->scaling.setDescription(
                    textIdTr("Scale entities according some factor"));
        this->importAnnotations.setDescription(
                    textIdTr("Import text/dimension objects"));
        this->groupLayers.setDescription(
                    textIdTr("Group all objects within a layer into a single compound shape"));
        this->fontNameForTextObjects.setDescription(
                    textIdTr("Name of the font to be used when creating shape for text objects"));
    }

    void restoreDefaults() override {
        const DxfReader::Parameters params;
        this->scaling.setValue(params.scaling);
        this->importAnnotations.setValue(params.importAnnotations);
        this->groupLayers.setValue(params.groupLayers);
        this->fontNameForTextObjects.setValue(0);
    }

    PropertyDouble scaling{ this, textId("scaling") };
    PropertyBool importAnnotations{ this, textId("importAnnotations") };
    PropertyBool groupLayers{ this, textId("groupLayers") };
    PropertyEnumeration fontNameForTextObjects{ this, textId("fontNameForTextObjects"), &systemFontNames() };
};

DxfReader::~DxfReader()
{
    delete m_internal;
}

bool DxfReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    delete m_internal;
    m_internal = new DxfReader::Internal;
    m_internal->setParameters(m_params);
    m_internal->setMessenger(this->messenger() ? this->messenger() : &Messenger::null());
    return m_internal->read(filepath, progress);
}

std::string getEntityName(const Dxf_EntityVariant& entityVar)
{
    using namespace std::string_literals;
    return std::visit(Cpp::Overloaded{
        [](std::monostate) { return std::string{}; },
        [=](ConstRefWrap<Dxf_3DFACE>) { return "3DFACE"s; },
        [=](ConstRefWrap<Dxf_ARC>) { return "ARC"s; },
        [=](ConstRefWrap<Dxf_CIRCLE>) { return "CIRCLE"s; },
        [=](ConstRefWrap<Dxf_ELLIPSE>) { return "ELLIPSE"s; },
        [=](ConstRefWrap<Dxf_INSERT> obj) { return "INSERT_" + std::string{obj.get().blockName}; },
        [=](ConstRefWrap<Dxf_LINE>) { return "LINE"s; },
        [=](ConstRefWrap<Dxf_LWPOLYLINE>) { return "LWPOLYLINE"s; },
        [=](ConstRefWrap<Dxf_MTEXT>) { return "MTEXT"s; },
        [=](ConstRefWrap<Dxf_POINT>) { return "POINT"s; },
        [=](ConstRefWrap<Dxf_POLYLINE>) { return "POLYLINE"s; },
        [=](ConstRefWrap<Dxf_SOLID>) { return "SOLID"s; },
        [=](ConstRefWrap<Dxf_SPLINE>) { return "SPLINE"s; },
        [=](ConstRefWrap<Dxf_TEXT>) { return "TEXT"s; }
        }, entityVar
    );
}

DxfStringRef getEntityLayerName(const Dxf_EntityVariant& entityVar)
{
    return std::visit([](auto&& arg) -> std::string_view {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>)
                return {};
            else
                return arg.get().layerName;
        }, entityVar
    );
}

TDF_LabelSequence DxfReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (!m_internal)
        return {};

    TDF_LabelSequence seqLabel;
    OccHandle<XCAFDoc_ShapeTool> shapeTool = doc->xcaf().shapeTool();
    OccHandle<XCAFDoc_ColorTool> colorTool = doc->xcaf().colorTool();
    OccHandle<XCAFDoc_LayerTool> layerTool = doc->xcaf().layerTool();
    std::unordered_map<DxfStringRef, TDF_Label> mapLayerNameLabel;
    std::unordered_map<DxfColorIndex, TDF_Label> mapAciColorLabel;

    auto fnAddRootShape = [&](const TopoDS_Shape& shape, std::string_view shapeName, TDF_Label layer) {
        const TDF_Label labelShape = shapeTool->NewShape();
        shapeTool->SetShape(labelShape, shape);
        TDataStd_Name::Set(labelShape, to_OccExtString(shapeName));
        seqLabel.Append(labelShape);
        if (!layer.IsNull())
            layerTool->SetLayer(labelShape, layer, true/*onlyInOneLayer*/);

        return labelShape;
    };

    auto fnAddAci = [&](DxfColorIndex aci) -> TDF_Label {
        auto it = mapAciColorLabel.find(aci);
        if (it != mapAciColorLabel.cend())
            return it->second;

        if (0 <= aci && Cpp::cmpLess(aci, std::size(aciTable))) {
            const RGB_Color& c = aciTable[aci].second;
            const TDF_Label colorLabel = colorTool->AddColor(
                Quantity_Color(c.r / 255., c.g / 255., c.b / 255., Quantity_TOC_RGB)
            );
            mapAciColorLabel.insert({ aci, colorLabel });
            return colorLabel;
        }

        return TDF_Label{};
    };

    unsigned iShape = 0;
    for (const auto& [layerName, vecEntity] : m_layers) {
    if (m_params.groupLayers) {
        std::unordered_map<const Dxf_LAYER*, TopoDS_Shape> mapShapeByLayer;
        for (const Dxf_EntityVariant& entityVar : m_internal->allEntities()) {
            const TopoDS_Shape entityShape = m_internal->createEntityShape(entityVar);
            if (entityShape.IsNull())
                continue; // Skip

            DxfStringRef layerName = getEntityLayerName(entityVar);
            const Dxf_LAYER* layer = m_internal->findLayer(layerName);
            auto it = mapShapeByLayer.find(layer);
            if (it == mapShapeByLayer.cend()) {
                TopoDS_Shape layerShape = BRepUtils::makeEmptyCompound();
                it = mapShapeByLayer.insert({ layer, layerShape }).first;
            }

            assert(it != mapShapeByLayer.cend());
            TopoDS_Shape& layerShape = it->second;
            BRepUtils::addShape(&layerShape, entityShape);
        }

        for (const auto& [layer, shape] : mapShapeByLayer) {
            DxfStringRef layerName = layer ? layer->name : std::string_view{"0"};
            TDF_Label label = layerTool->AddLayer(to_OccExtString(layerName));
            fnAddRootShape(shape, layerName, label);
        }
    }
    else {
        std::unordered_map<size_t, unsigned> mapCountByEntityType;
        for (const Dxf_EntityVariant& entityVar : m_internal->allEntities()) {
            const TopoDS_Shape entityShape = m_internal->createEntityShape(entityVar);
            if (entityShape.IsNull())
                continue; // Skip

            // Entity name
            std::string strEntityName = getEntityName(entityVar);
            {
                unsigned iShape = 1;
                auto it = mapCountByEntityType.find(entityVar.index());
                if (it == mapCountByEntityType.cend())
                    mapCountByEntityType.insert({entityVar.index(), 1});
                else
                    iShape = ++(it->second);

                strEntityName += "_" + std::to_string(iShape);
            }

            // Move entity in layer
            TDF_Label layerLabel;
            {
                DxfStringRef layerName = getEntityLayerName(entityVar);
                auto it = mapLayerNameLabel.find(getEntityLayerName(entityVar));
                if (it == mapLayerNameLabel.cend()) {
                    layerLabel = layerTool->AddLayer(to_OccExtString(layerName));
                    mapLayerNameLabel.insert({ layerName, layerLabel });
                }
                else {
                    layerLabel = it->second;
                }
            }

            fnAddRootShape(entityShape, strEntityName, layerLabel);
        }
    }

    return seqLabel;
#if 0
    int iShape = 0;
    int shapeCount = 0;
    for (const auto& [layerName, vecEntity] : m_internal->layers()) {
        if (!startsWith(layerName, "BLOCKS")) {
            shapeCount += static_cast<unsigned>(vecEntity.size());
            const TDF_Label layerLabel = layerTool->AddLayer(to_OccExtString(layerName));
            mapLayerNameLabel.insert({ layerName, layerLabel });
        }
    }
    auto fnUpdateProgressValue = [&]{
        progress->setValue(MathUtils::toPercent(iShape, 0u, shapeCount));
    };

    auto fnSetShapeColor = [=](const TDF_Label& labelShape, int aci) {
        const TDF_Label labelColor = fnAddAci(aci);
        if (!labelColor.IsNull())
            colorTool->SetColor(labelShape, labelColor, XCAFDoc_ColorGen);
    };

    if (!m_params.groupLayers) {
        for (const auto& [layerName, vecEntity] : m_internal->layers()) {
            if (startsWith(layerName, "BLOCKS"))
                continue; // Skip

            const TDF_Label layerLabel = Cpp::findValue(layerName, mapLayerNameLabel);
            for (const DxfReader::Entity& entity : vecEntity) {
                const std::string shapeName = std::string("Shape_") + std::to_string(++iShape);
                const TDF_Label shapeLabel = fnAddRootShape(entity.shape, shapeName, layerLabel);
                colorTool->SetColor(shapeLabel, fnAddAci(entity.aci), XCAFDoc_ColorGen);
                fnUpdateProgressValue();
            }
        }
    }
    else {
        for (const auto& [layerName, vecEntity] : m_internal->layers()) {
            if (startsWith(layerName, "BLOCKS"))
                continue; // Skip

            TopoDS_Compound comp = BRepUtils::makeEmptyCompound();
            for (const Entity& entity : vecEntity) {
                if (!entity.shape.IsNull())
                    BRepUtils::addShape(&comp, entity.shape);
            }

            if (!comp.IsNull()) {
                const TDF_Label layerLabel = Cpp::findValue(layerName, mapLayerNameLabel);
                const TDF_Label compLabel = fnAddRootShape(comp, layerName, layerLabel);
                // Check if all entities have the same color
                bool uniqueColor = true;
                const ColorIndex_t aci = !vecEntity.empty() ? vecEntity.front().aci : -1;
                for (const Entity& entity : vecEntity) {
                    uniqueColor = entity.aci == aci;
                    if (!uniqueColor)
                        break;
                }

                if (uniqueColor) {
                    fnSetShapeColor(compLabel, aci);
                }
                else {
                    for (const Entity& entity : vecEntity) {
                        if (!entity.shape.IsNull()) {
                            const TDF_Label entityLabel = shapeTool->AddSubShape(compLabel, entity.shape);
                            fnSetShapeColor(entityLabel, entity.aci);
                        }
                    }
                }
            }

            iShape += static_cast<unsigned>(vecEntity.size());
            fnUpdateProgressValue();
        }
    }

    return seqLabel;
#endif
}

std::unique_ptr<PropertyGroup> DxfReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void DxfReader::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.scaling = ptr->scaling;
        m_params.importAnnotations = ptr->importAnnotations;
        m_params.groupLayers = ptr->groupLayers;
        m_params.fontNameForTextObjects = ptr->fontNameForTextObjects.valueName();
    }
}

bool DxfReader::Internal::read(const FilePath& filepath, TaskProgress* progress)
{
    std::ifstream fstr(filepath);
    if (!fstr.is_open())
        return false;

    m_fileSize = filepathFileSize(filepath);
    m_progress = progress;
    CDxfRead::read(fstr);
    return !this->failed();
}

void DxfReader::Internal::getLine()
{
    CDxfRead::getLine();
    ++m_lineCounter;
    m_fileReadSize += this->gcount();
    if (m_progress)
        m_progress->setValue(MathUtils::toPercent(m_fileReadSize, 0, m_fileSize));
}

bool DxfReader::Internal::setSourceEncoding(const std::string& codepage)
{
    std::optional<Resource_FormatType> encoding;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    if (codepage == "UTF8")
        encoding = Resource_FormatType_UTF8;
    else if (codepage == "ANSI_932") // Japanese
        encoding = Resource_FormatType_SJIS;
    else if (codepage == "ANSI_936") // UnifiedChinese
        encoding = Resource_FormatType_GB;
    else if (codepage == "ANSI_949") // Korean
        encoding = Resource_FormatType_EUC;
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    if (encoding)
        ; // Encoding already found above
    else if (codepage == "ANSI_936") // UnifiedChinese
        encoding = Resource_FormatType_GBK;
    else if (codepage == "ANSI_950") // TradChinese
        encoding = Resource_FormatType_Big5;
    else if (codepage == "ANSI_1250")
        encoding = Resource_FormatType_CP1250;
    else if (codepage == "ANSI_1251")
        encoding = Resource_FormatType_CP1251;
    else if (codepage == "ANSI_1252")
        encoding = Resource_FormatType_CP1252;
    else if (codepage == "ANSI_1253")
        encoding = Resource_FormatType_CP1253;
    else if (codepage == "ANSI_1254")
        encoding = Resource_FormatType_CP1254;
    else if (codepage == "ANSI_1255")
        encoding = Resource_FormatType_CP1255;
    else if (codepage == "ANSI_1256")
        encoding = Resource_FormatType_CP1256;
    else if (codepage == "ANSI_1257")
        encoding = Resource_FormatType_CP1257;
    else if (codepage == "ANSI_1258")
        encoding = Resource_FormatType_CP1258;
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    if (encoding)
        ; // Encoding already found above
    else if (codepage == "ANSI_850" || codepage == "DOS850")
        encoding = Resource_FormatType_CP850;
#endif

    if (encoding) {
        m_srcEncoding = encoding.value();
    }
    else {
        m_srcEncoding = Resource_ANSI;
        m_messenger->emitWarning("Codepage " + codepage + " not supported");
    }

    return true;
}

std::string DxfReader::Internal::toUtf8(const std::string& strSource)
{
    if (m_srcEncoding == Resource_ANSI) // Resource_ANSI is a pass-through(OpenCascade)
        return strSource;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    if (m_srcEncoding == Resource_FormatType_UTF8)
        return strSource;

    TCollection_ExtendedString extStr;
    Resource_Unicode::ConvertFormatToUnicode(m_srcEncoding, strSource.c_str(), extStr);
    return to_stdString(extStr);
#else
    return strSource;
#endif
}

TopoDS_Shape DxfReader::Internal::createBlockShape(const Dxf_BLOCK& block)
{
    static const OccBlock defaultOccBlock{ BlockState::Unvisited, TopoDS_Shape{} };
    auto [it, inserted] = m_mapOccBlock.try_emplace(block.name, defaultOccBlock);
    OccBlock& occBlock = it->second;
    switch (occBlock.state) {
    case BlockState::Resolved:
        return occBlock.shape;
    case BlockState::Visiting:
        throw std::runtime_error("Cyclic block reference detected");
        break;
    case BlockState::Unvisited:
        occBlock.state = BlockState::Visiting;
        break;
    }

    TopoDS_Shape blockShape = BRepUtils::makeEmptyCompound();
    for (const Dxf_EntityVariant& entityVar : block.entities)
        BRepUtils::addShape(&blockShape, this->createEntityShape(entityVar));

    occBlock.state = BlockState::Resolved;
    occBlock.shape = blockShape;
    return blockShape;
}

TopoDS_Shape DxfReader::Internal::createEntityShape(const Dxf_EntityVariant& entityVar)
{
    const TopoDS_Shape entityShape = std::visit(Cpp::Overloaded{
        [](std::monostate) { return TopoDS_Shape{}; },
        [=](ConstRefWrap<Dxf_3DFACE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_ARC> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_CIRCLE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_ELLIPSE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_INSERT> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_LINE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_LWPOLYLINE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_MTEXT> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_POINT> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_POLYLINE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_SOLID> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_SPLINE> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_TEXT> obj) { return createShape(obj); }
    }, entityVar);
    return entityShape;
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_3DFACE& face)
{
    try {
        return makeFace(face);
    }
    catch (...) {
        m_messenger->emitError("createShape(3DFACE) failed");
        return {};
    }
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_ARC& arc)
{
    if (arc.radius <= Precision::Confusion()) {
        m_messenger->emitError("ARC radius is null or degenerated");
        return {};
    }

    const Placement pl = makePlacementFromOcs(arc.centerPoint, arc.extrusionDirection);

    const gp_Circ circ{pl.ax2, arc.radius};

    const double a1d = normalizeAngleDeg(arc.startAngle);
    const double a2d = normalizeAngleDeg(arc.endAngle);
    if (std::abs(a1d - a2d) <= Precision::Angular())
        return {};

    const double a1 = MathUtils::degreeToRadian(a1d);
    const double a2 = MathUtils::degreeToRadian(a2d);

    const gp_Pnt& c = pl.ax2.Location();
    const gp_Pnt p1 = pointOnCircle(c, arc.radius, a1, pl.frame.u, pl.frame.v);
    const gp_Pnt p2 = pointOnCircle(c, arc.radius, a2, pl.frame.u, pl.frame.v);
    return makeExtrusionShape(BRepBuilderAPI_MakeEdge(circ, p1, p2), arc.thickness, pl.frame.w);
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_CIRCLE& circle)
{
    if (circle.radius <= Precision::Confusion()) {
        m_messenger->emitError("CIRCLE radius is null or quasi null");
        return {};
    }

    const Placement pl = makePlacementFromOcs(circle.centerPoint, circle.extrusionDirection);
    const gp_Circ circ{pl.ax2, circle.radius};
    return makeExtrusionShape(BRepBuilderAPI_MakeEdge(circ), circle.thickness, pl.frame.w);
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_ELLIPSE& ellipse)
{
    if (ellipse.ratioMinorMajorAxis <= Precision::Confusion()) {
        m_messenger->emitError("ELLIPSE ratio is null or quasi null");
        return {};
    }

    Placement pl = makePlacementFromOcs(ellipse.centerPoint, ellipse.extrusionDirection);

    const gp_Vec majorw =
        gp_Vec{pl.frame.u} * ellipse.majorAxisEndPoint.x
        + gp_Vec{pl.frame.v} * ellipse.majorAxisEndPoint.y
        + gp_Vec{pl.frame.w} * ellipse.majorAxisEndPoint.z
    ;
    const double rmajor = majorw.Magnitude();
    if (rmajor <= Precision::Confusion()) {
        m_messenger->emitError("ELLIPSE major axis is null or quasi null");
        return {};
    }

    const double rminor = ellipse.ratioMinorMajorAxis * rmajor;
    if (rminor <= Precision::Confusion()) {
        m_messenger->emitError("ELLIPSE minor axis is null or quasi null");
        return {};
    }

    pl.ax2.SetXDirection(gp_Dir{majorw});

    const gp_Elips elips(pl.ax2, rmajor, rminor);

    // Build edge, complete or partial
    TopoDS_Edge edge;
    const double u1 = ellipse.startParam;
    const double u2 = ellipse.endParam;
    if (std::abs(u1 - u2) <= Precision::Angular())
        edge = BRepBuilderAPI_MakeEdge(elips);
    else
        edge = BRepBuilderAPI_MakeEdge(elips, u1, u2);

    return makeExtrusionShape(BRepBuilderAPI_MakeEdge(elips), ellipse.thickness, pl.frame.w);
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_INSERT& insert)
{
    if (!insert.isVisible)
        return {};

    const Dxf_BLOCK* blockPtr = this->findBlock(insert.blockName);
    if (!blockPtr)
        return {}; // TODO Emit error message

    const Dxf_BLOCK& block = *blockPtr;

    // Shape for block content(block frame, non transformed)
    TopoDS_Shape content = this->createBlockShape(block);
    if (content.IsNull())
        return {};

    // OCS frame of the INSERT
    const Frame frame = makeOcsFrame(toOccDir(insert.extrusionDirection));

    // Parameters for grid-like insertion
    const int rows = std::max(1, insert.rowCount);
    const int cols = std::max(1, insert.columnCount);

    // Check if uniform scale
    const bool unitScale = isUnitScale(insert.scaleFactor);

    // Create shape for the result
    TopoDS_Compound result = BRepUtils::makeEmptyCompound();

    // Common pre-computations
    const gp_Pnt insertPnt_wcs = ocsPointToWcs(insert.insertPoint, frame);
    const gp_Vec blockPnt_wcs = ocsVecToWcs(block.basePoint, frame);
    const double theta = MathUtils::degreeToRadian(insert.rotationAngle);
    const double rowSpacing = insert.rowSpacing;
    const double colSpacing = insert.columnSpacing;
    const gp_Vec uframe{frame.u};
    const gp_Vec vframe{frame.v};

    for (int j = 0; j < rows; ++j) {
        for (int i = 0; i < cols; ++i) {
            // Grid offset along frame u/v
            const gp_Vec gridOffset = uframe * (rowSpacing * i) + vframe * (colSpacing * j);
            const gp_Pnt Pij = insertPnt_wcs.Translated(gridOffset);

            TopoDS_Shape instance;

            if (unitScale) {
                const gp_Trsf scaleTrsf = unitScaleTrsf(gp::Origin(), frame, insert.scaleFactor);
                const gp_Trsf rotationTrsf = GeomUtils::makeRotation(gp_Ax1{gp::Origin(), frame.w}, theta);
                const gp_Trsf t1Trsf = GeomUtils::makeTranslation(-blockPnt_wcs);
                const gp_Trsf t2Trsf = GeomUtils::makeTranslation(Pij.XYZ());
                const gp_Trsf trsf = t2Trsf * rotationTrsf * scaleTrsf * t1Trsf;
                instance = content.Moved(trsf);
            } else {
                // Non-unit scale -> GTransform

                // U = [u v w]
                const gp_Mat matFrame{
                    frame.u.X(), frame.v.X(), frame.w.X(),
                    frame.u.Y(), frame.v.Y(), frame.w.Y(),
                    frame.u.Z(), frame.v.Z(), frame.w.Z()
                };
                const DxfScale& sf = insert.scaleFactor;
                const gp_Mat matScale{
                    sf.x, 0,    0,
                    0,    sf.y, 0,
                    0,    0,    sf.z
                };
                const gp_Mat Ms = matFrame * matScale * matFrame.Transposed();
                const gp_Trsf R = GeomUtils::makeRotation(gp_Ax1{gp::Origin(), frame.w}, theta);
                const gp_Mat L = R.VectorialPart() * Ms;

                const gp_XYZ LB = L * blockPnt_wcs.XYZ();
                const gp_XYZ tXYZ = Pij.XYZ() - LB;

                gp_GTrsf G;
                G.SetVectorialPart(L);
                G.SetTranslationPart(tXYZ);

                instance = BRepBuilderAPI_GTransform(content, G, true/*copy*/).Shape();
            }

            BRepUtils::addShape(&result, instance);
        }
    }

    return result;
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_LINE& line)
{
    const Frame frame = makeOcsFrame(toOccDir(line.extrusionDirection));
    const gp_Pnt p1 = ocsPointToWcs(line.startPoint, frame);
    const gp_Pnt p2 = ocsPointToWcs(line.endPoint, frame);
    if (p1.SquareDistance(p2) <= Precision::SquareConfusion())
        return {};

    return makeExtrusionShape(BRepBuilderAPI_MakeEdge(p1, p2), line.thickness, frame.w);
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_LWPOLYLINE& polyline)
{
    // Create OCC curve corresponding to arc via bulge
    auto makeArcFromBulge = [](
            const gp_Pnt& p0, const gp_Pnt& p1, double bulge, const gp_Dir& planeNormal
        ) -> OccHandle<Geom_TrimmedCurve>
    {
        gp_Vec chord(p0, p1);
        const double c = chord.Magnitude();
        if (c <= gp::Resolution() || MathUtils::fuzzyIsNull(bulge))
            return {};

        gp_Vec p = gp_Vec{planeNormal} ^ chord.Normalized(); // normal × chord
        if (p.SquareMagnitude() <= Precision::SquareConfusion())
            return {};

        p.Normalize();

        // Sagitta s = (bulge * c) / 2
        const double s = (bulge * c) * 0.5;
        const gp_Pnt m = p0.Translated(chord * 0.5);
        return GC_MakeArcOfCircle(p0, m.Translated(p * s), p1);
    };

    const size_t n = polyline.vertices.size();
    if (n < 2)
        return {};

    const bool isClosed = (polyline.flag & 1u) != 0u;
    const Placement pl = makePlacementFromOcs({0., 0., polyline.elevation}, polyline.extrusionDirection);
    const gp_Dir& normal = pl.ax2.Direction();

    auto makePolar = [&](const Dxf_LWPOLYLINE::Vertex& vertex) -> gp_Pnt {
        const gp_Vec vec = gp_Vec{pl.frame.u} * vertex.x + gp_Vec{pl.frame.v} * vertex.y;
        return pl.ax2.Location().Translated(vec);
    };

    BRepBuilderAPI_MakeWire wireBuilder;
    for (size_t i = 0; i + 1 < n; ++i) {
        const auto& v0 = polyline.vertices.at(i);
        const auto& v1 = polyline.vertices.at(i+1);
        const gp_Pnt p0 = makePolar(v0);
        const gp_Pnt p1 = makePolar(v1);

        if (MathUtils::fuzzyIsNull(v0.bulge)) {
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(p0, p1));
        } else {
            OccHandle<Geom_TrimmedCurve> arc = makeArcFromBulge(p0, p1, v0.bulge, normal);
            if (!arc.IsNull())
                wireBuilder.Add(BRepBuilderAPI_MakeEdge(arc));
            else
                wireBuilder.Add(BRepBuilderAPI_MakeEdge(p0, p1));
        }
    }

    if (isClosed) {
        const auto& vL = polyline.vertices.at(n - 1);
        const auto& vF = polyline.vertices.at(0);
        const gp_Pnt p0 = makePolar(vL);
        const gp_Pnt p1 = makePolar(vF);

        if (MathUtils::fuzzyIsNull(vL.bulge)) {
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(p0, p1));
        } else {
            OccHandle<Geom_TrimmedCurve> arc = makeArcFromBulge(p0, p1, vL.bulge, normal);
            if (arc)
                wireBuilder.Add(BRepBuilderAPI_MakeEdge(arc));
            else
                wireBuilder.Add(BRepBuilderAPI_MakeEdge(p0, p1));
        }
    }

    if (!wireBuilder.IsDone())
        return {}; // TODO Emit error message?

#if 0
    if (isClosed) {
        const gp_Pln plane(pl.ax2.Location(), normal);
        BRepBuilderAPI_MakeFace faceBuilder(plane, wireBuilder.Wire(), true/*inside*/);
        if (faceBuilder.IsDone())
            return makeExtrusionShape(faceBuilder.Face(), polyline.thickness, pl.frame.w);
    }
#endif

    return makeExtrusionShape(wireBuilder.Wire(), polyline.thickness, pl.frame.w);
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_MTEXT& mtext)
{
    if (!m_params.importAnnotations)
        return {};

    const gp_Pnt pt = toOccPnt(mtext.insertionPoint);
    const std::string& fontName = m_params.fontNameForTextObjects;
    const double fontHeight = 1.4 * mtext.height * m_params.scaling;
    Font_BRepFont brepFont;
    if (!brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight)) {
        m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        return {};
    }

    const int ap = static_cast<int>(mtext.attachmentPoint);
    Graphic3d_HorizontalTextAlignment hAlign = Graphic3d_HTA_LEFT;
    if (ap == 2 || ap == 5 || ap == 8)
        hAlign = Graphic3d_HTA_CENTER;
    else if (ap == 3 || ap == 6 || ap == 9)
        hAlign = Graphic3d_HTA_RIGHT;

    Graphic3d_VerticalTextAlignment vAlign = Graphic3d_VTA_TOP;
    if (ap == 4 || ap == 5 || ap == 6)
        vAlign = Graphic3d_VTA_CENTER;
    else if (ap == 7 || ap == 8 || ap == 9)
        vAlign = Graphic3d_VTA_BOTTOM;

    // Ensure non-null x-axis direction
    gp_Vec xAxisDir = toOccVec(mtext.xAxisDirection);
    if (xAxisDir.Magnitude() < gp::Resolution())
        xAxisDir = gp::DX();

    // If rotation angle is non-null and x-axis direction defaults to standard Ox then set x-axis
    // so it matches rotation angle
    xAxisDir.Normalize();
    if (!MathUtils::fuzzyIsNull(mtext.rotationAngle)
        && xAxisDir.IsEqual(gp::DX(), Precision::Confusion(), Precision::Angular()))
    {
        const double angle = MathUtils::degreeToRadian(mtext.rotationAngle);
        const gp_Trsf trsf = GeomUtils::makeRotation(gp_Ax1(pt, gp::DZ()), angle);
        xAxisDir = gp::DX().Transformed(trsf);
    }

    const auto occTextStr = string_conv<NCollection_String>(mtext.str);
    const gp_Dir extDir = toOccDir(mtext.extrusionDirection);
    const gp_Ax3 locText(pt, extDir, xAxisDir);
    Font_BRepTextBuilder brepTextBuilder;
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    auto textFormat = makeOccHandle<Font_TextFormatter>();
    textFormat->SetupAlignment(hAlign, vAlign);
    textFormat->Append(occTextStr, *brepFont.FTFont());
    /* Font_TextFormatter computes weird ResultWidth() so wrapping is currently broken
    if (text.acadHasColumnInfo && text.acadColumnInfo_Width > 0.) {
        textFormat->SetWordWrapping(true);
        textFormat->SetWrapping(text.acadColumnInfo_Width);
    }
    */
    textFormat->Format();
    return brepTextBuilder.Perform(brepFont, textFormat, locText);
#else
    return brepTextBuilder.Perform(brepFont, occTextStr, locText, hAlign, vAlign);
#endif
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_POINT& point)
{
    TopoDS_Shape shape = BRepUtils::makeEmptyCompound();

    const gp_Pnt pnt = toOccPnt(point.location);
    const gp_Dir n = toOccDir(point.extrusionDirection);
    // If thickness != 0 → extruded segment
    if (!MathUtils::fuzzyIsNull(point.thickness)) {
        const gp_Vec step(n.XYZ() * point.thickness);
        const gp_Pnt dst(pnt.X() + step.X(), pnt.Y() + step.Y(), pnt.Z() + step.Z());
        if (pnt.Distance(dst) > Precision::Confusion())
            BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(pnt, dst));
    }

    const int pdmode = dxfGetInt(this->headerVariableValue("PDMODE"), 0);
    const double pdsize = dxfGetDouble(this->headerVariableValue("PDSIZE"), 0.);

    // Interpret PDMODE/PDSIZE variables
    const bool drawCircle = (pdmode & 32) != 0;
    const bool drawSquare = (pdmode & 64) != 0;
    const int base = pdmode & 0x1F; // 0..31; base values 0,1,2,3,4

    gp_Ax2 ax2 = makePlacementFromOcs(point.location, point.extrusionDirection).ax2;
    const double angleXAxis = pdmode != 0 ? point.angleXAxis : 0.;
    if (!MathUtils::fuzzyIsNull(point.angleXAxis)) {
        const double angleXAxis_rad = MathUtils::degreeToRadian(angleXAxis);
        ax2.Transform(GeomUtils::makeRotation(gp_Ax1(pnt, ax2.Direction()), angleXAxis_rad));
    }

    // Symbol size according PDSIZE

    // Resolve symbol size accoding PDSIZE and an optional viewport height
    // - pdsize > 0 : absolute size -> returned as is
    // - pdsize == 0 : 5% of viewportHeight if provided, fallback(1.0) otherwise
    // - pdsize < 0 : (-pdsize)% of viewportHeight if provided, fallback(|pdsize|) otherwise
    auto resolvePointSizeInModelUnits = [](
            double pdsize, std::optional<double> viewportHeight, double fallbackAbsSize = 1.0
        )
    {
        if (pdsize > 0.)
            return pdsize;

        const double vpHeight = viewportHeight ? *viewportHeight : -1.;
        if (MathUtils::fuzzyIsNull(pdsize))
            return vpHeight > 0. ? 0.05 * vpHeight : fallbackAbsSize;

        // pdsize < 0
        if (vpHeight > 0.)
            return (std::abs(pdsize) / 100.) * vpHeight;

        // Return fallback value when viewport is unknown
        return std::max(fallbackAbsSize, std::abs(pdsize));
    };

    // Build segment[-len/2, +len/2] along `dir`
    auto makeCenteredSegment = [](const gp_Pnt& pnt, const gp_Dir& dir, double len) {
        const gp_Vec u{dir};
        const gp_Pnt p1 = pnt.Translated(-0.5 * len * u);
        const gp_Pnt p2 = pnt.Translated( 0.5 * len * u);
        return BRepBuilderAPI_MakeEdge(p1, p2);
    };

    const std::optional<double> viewportHeight = std::nullopt;
    const double fallbackAbsSize = 1.;

    // Symbol size according PDSIZE
    const double L = resolvePointSizeInModelUnits(pdsize, viewportHeight, fallbackAbsSize);

    // Special cases for PDMODE:
    //   1 => none(display nothing): if no thickness then return null shape
    //   0 => dot: add vertex
    if (base == 1)
        return shape;

    if (base == 0) {
        BRepUtils::addShape(&shape, BRepBuilderAPI_MakeVertex(pnt));
    }
    else {
        // 2, 3, 4 : figures "linéaires" orientées par le code 50
        switch (base) {
        case 2: // cross '+'
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), ax2.XDirection(), L));
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), ax2.YDirection(), L));
            break;
        case 3: { // x-cross 'x'
            const gp_Vec u{ax2.XDirection()};
            const gp_Vec v{ax2.YDirection()};
            const gp_Vec diagPlus = u.Added(v).Normalized();
            const gp_Vec diagMinus = u.Subtracted(v).Normalized();
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), diagPlus, L));
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), diagMinus, L));
        }
            break;
        case 4: // tick '''
            // Small "dash" oriented along angle(code 50), centered on POINT location
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), ax2.XDirection(), 0.75 * L));
            break;
        default:
            // Fallback = vertex
            BRepUtils::addShape(&shape, BRepBuilderAPI_MakeVertex(pnt));
            break;
        }
    }

    // Additional stuff: circle / square
    if (drawCircle) {
        // Radius = L/2
        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(gp_Circ(ax2, 0.5 * L));
        BRepUtils::addShape(&shape, BRepBuilderAPI_MakeWire(edge));
    }

    if (drawSquare) {
        const gp_Vec ux{ax2.XDirection()};
        const gp_Vec vy{ax2.YDirection()};
        const double h = 0.5 * L;

        const gp_Pnt p1 = ax2.Location().Translated((-h)*ux + (-h)*vy);
        const gp_Pnt p2 = ax2.Location().Translated(( h)*ux + (-h)*vy);
        const gp_Pnt p3 = ax2.Location().Translated(( h)*ux + ( h)*vy);
        const gp_Pnt p4 = ax2.Location().Translated((-h)*ux + ( h)*vy);

        BRepBuilderAPI_MakeWire wireBuilder;
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2));
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2, p3));
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(p3, p4));
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(p4, p1));
        BRepUtils::addShape(&shape, wireBuilder.Wire());
    }

    return shape;
}

TopoDS_Shape DxfReader::Internal::createShapePolygonMesh3d(const Dxf_POLYLINE& polyline)
{
    const auto& vertices = polyline.vertices;
    // MxN regular mesh
    const int meshM = polyline.polygonMeshMVertexCount;
    const int meshN = polyline.polygonMeshNVertexCount;
    const bool isMeshMClosed = (polyline.flags & Dxf_POLYLINE::Flag::Closed) != 0;
    const bool isMeshNClosed = (polyline.flags & Dxf_POLYLINE::Flag::PolygonMeshClosedNDir) != 0;
    if (meshM <= 0 || meshN <= 0)
        return {};

    if (int(vertices.size()) < meshM * meshN)
        return {}; // Incomplete

    int triCount = 0;
    if (meshM >= 2 && meshN >= 2) {
        triCount = 2 * (meshM - 1) * (meshN - 1); // Central grid
        if (isMeshMClosed)
            triCount += 2 * (meshN - 1);

        if (isMeshNClosed)
            triCount += 2 * (meshM - 1);

        if (isMeshMClosed && isMeshNClosed)
            triCount += 2;
    }

    const int nodeCount = meshM * meshN;
    if (triCount == 0)
        return {};

    // i in [0..meshM-1], j in [0..meshN-1]
    auto nodeIndex = [=](int i, int j) {
        return j * meshM + i + 1;
    };

    TColgp_Array1OfPnt nodes(1, nodeCount);
    // Vertices are listed line-by-line
    for (int j = 0; j < meshN; ++j) {
        for (int i = 0; i < meshM; ++i) {
            const int index = nodeIndex(i, j);
            nodes.ChangeValue(index) = toOccPnt(vertices.at(index-1).point);
        }
    }

    int ktri = 1; // Current triangle index(1..triCount)
    Poly_Array1OfTriangle triangles(1, triCount);
    auto addTri = [&](int i1, int i2, int i3) {
        triangles.ChangeValue(ktri++) = Poly_Triangle{i1, i2, i3};
    };
    auto addQuad = [&](int p00, int p10, int p01, int p11) {
        // Quad = two triangles p00-p10-p11 and p00-p11-p01
        addTri(p00, p10, p11);
        addTri(p00, p11, p01);
    };

    // Triangulation of "inferior" grid(without closure)
    if (meshM >= 2 && meshN >= 2) {
        for (int j = 0; j < meshN - 1; ++j) {
            for (int i = 0; i < meshM - 1; ++i)
                addQuad(nodeIndex(i, j), nodeIndex(i+1, j), nodeIndex(i, j+1), nodeIndex(i+1, j+1));
        }
    }

    // M closure: column M-1 connected to column 0
    if (isMeshMClosed && meshN >= 2) {
        for (int j = 0; j < meshN - 1; ++j)
            addQuad(nodeIndex(meshM-1, j), nodeIndex(0, j), nodeIndex(meshM-1, j+1), nodeIndex(0, j+1));
    }

    // N closure: line N-1 connected to line 0
    if (isMeshNClosed && meshM >= 2) {
        for (int i = 0; i < meshM - 1; ++i)
            addQuad(nodeIndex(i, meshN-1), nodeIndex(i+1, meshN-1), nodeIndex(i, 0), nodeIndex(i+1, 0));
    }

    // Corner if M and N closed
    if (isMeshMClosed && isMeshNClosed)
        addQuad(nodeIndex(meshM-1, meshN-1), nodeIndex(0, meshN-1), nodeIndex(meshM-1, 0), nodeIndex(0, 0));

    // Sanity check
    assert((ktri - 1) == triCount);

    return BRepUtils::makeFace(new Poly_Triangulation(nodes, triangles));
}

TopoDS_Shape DxfReader::Internal::createShapePolyfaceMesh(const Dxf_POLYLINE& polyline)
{
    const auto& vertices = polyline.vertices;
    const int meshVertexCount = polyline.polygonMeshMVertexCount;
    TColgp_Array1OfPnt nodes(1, meshVertexCount);
    for (int i = 0; i < meshVertexCount; ++i)
        nodes.ChangeValue(i + 1) = toOccPnt(vertices.at(i).point);

    const int meshFaceCount = polyline.polygonMeshNVertexCount;
    std::vector<Poly_Triangle> vecTriangle;
    vecTriangle.reserve(meshFaceCount);
    for (int i = 0; i < meshFaceCount; ++i) {
        const Dxf_POLYLINE::Vertex& face = vertices.at(meshVertexCount + i);
        const auto meshVertex1 = std::abs(face.polyfaceMeshVertex1);
        const auto meshVertex2 = std::abs(face.polyfaceMeshVertex2);
        const auto meshVertex3 = std::abs(face.polyfaceMeshVertex3);
        const auto meshVertex4 = std::abs(face.polyfaceMeshVertex4);
        vecTriangle.emplace_back(meshVertex1, meshVertex2, meshVertex3);
        if (meshVertex4 != 0 && meshVertex3 != meshVertex4)
            vecTriangle.emplace_back(meshVertex1, meshVertex3, meshVertex4);
    }

    Poly_Array1OfTriangle triangles(1, static_cast<int>(vecTriangle.size()));
    for (unsigned i = 0; i < vecTriangle.size(); ++i)
        triangles.ChangeValue(i + 1) = vecTriangle.at(i);

    return BRepUtils::makeFace(new Poly_Triangulation(nodes, triangles));
}

TopoDS_Shape DxfReader::Internal::createShapePolyline3d(const Dxf_POLYLINE& polyline)
{
    const auto& vertices = polyline.vertices;
    const bool isPolylineClosed = (polyline.flags & Dxf_POLYLINE::Flag::Closed) != 0;
    const int nodeCount = gsl::narrow<int>(vertices.size() + (isPolylineClosed ? 1 : 0));
    MeshUtils::Polygon3dBuilder polygonBuilder(nodeCount);
    for (unsigned i = 0; i < vertices.size(); ++i)
        polygonBuilder.setNode(i + 1, toOccPnt(vertices.at(i).point));

    if (isPolylineClosed)
        polygonBuilder.setNode(nodeCount, toOccPnt(vertices.at(0).point));

    polygonBuilder.finalize();
    return BRepUtils::makeEdge(polygonBuilder.get());
}

TopoDS_Shape DxfReader::Internal::createShapePolyline2d(const Dxf_POLYLINE& polyline)
{
    // TODO Handle Dxf_POLYLINE::Vertex::bulge
    const auto& vertices = polyline.vertices;
    const bool isPolylineClosed = (polyline.flags & Dxf_POLYLINE::Flag::Closed) != 0;
    const int nodeCount = gsl::narrow<int>(vertices.size() + (isPolylineClosed ? 1 : 0));
    const gp_Dir extrusionDir = toOccDir(polyline.extrusionDirection);
    const Frame frame = makeOcsFrame(extrusionDir);
    MeshUtils::Polygon3dBuilder polygonBuilder(nodeCount);
    for (unsigned i = 0; i < vertices.size(); ++i)
        polygonBuilder.setNode(i + 1, ocsPointToWcs(vertices.at(i).point, frame));

    if (isPolylineClosed)
        polygonBuilder.setNode(nodeCount, ocsPointToWcs(vertices.at(0).point, frame));

    polygonBuilder.finalize();
    const TopoDS_Shape shape = BRepUtils::makeEdge(polygonBuilder.get());
    return makeExtrusionShape(shape, polyline.thickness, extrusionDir);
}

TopoDS_Shape DxfReader::Internal::createShapeCurveFit(const Dxf_POLYLINE& polyline)
{
    const auto& vertices = polyline.vertices;
    const bool isPolylineClosed = polyline.flags & Dxf_POLYLINE::Flag::Closed;
    const int nodeCount = gsl::narrow<int>(vertices.size() + (isPolylineClosed ? 1 : 0));
    const gp_Dir extrusionDir = toOccDir(polyline.extrusionDirection);
    const Frame frame = makeOcsFrame(extrusionDir);
    auto points = makeOccHandle<TColgp_HArray1OfPnt>(1, nodeCount);
    for (unsigned i = 0; i < vertices.size(); ++i)
        points->SetValue(i + 1, ocsPointToWcs(vertices.at(i).point, frame));

    if (isPolylineClosed)
        points->SetValue(nodeCount, ocsPointToWcs(vertices.at(0).point, frame));

    // BSpline interpolation
    // + Handle tangent directions at first and last vertices
    GeomAPI_Interpolate interp(points, false/*!periodic*/, Precision::Confusion());

    const Dxf_POLYLINE::Vertex& firstV = vertices.front();
    const Dxf_POLYLINE::Vertex& lastV = vertices.back();
    const bool hasTangentOnFirstV = (firstV.flags & Dxf_POLYLINE::Vertex::CurveFitTangent) != 0;
    const bool hasTangentOnLastV = (lastV.flags & Dxf_POLYLINE::Vertex::CurveFitTangent) != 0;
    if (hasTangentOnFirstV && hasTangentOnLastV) {
        auto dirFromAngle = [](double angle_deg, const Frame& frame) -> gp_Vec {
            const double a = MathUtils::degreeToRadian(angle_deg);
            const gp_Vec dir = std::cos(a)*gp_Vec(frame.u) + std::sin(a)*gp_Vec(frame.v);
            if (dir.SquareMagnitude() > Precision::SquareConfusion())
                return dir;
            else
                return frame.u;
        };
        const gp_Vec tanDirFirstV = dirFromAngle(firstV.curveFitTangentDirection, frame);
        const gp_Vec tanDirLastV = dirFromAngle(lastV.curveFitTangentDirection, frame);
        interp.Load(tanDirFirstV, tanDirLastV);
    }

    interp.Perform();
    if (!interp.IsDone())
        return {}; // TODO Emit error

    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(interp.Curve());
    return makeExtrusionShape(edge, polyline.thickness, extrusionDir);
}

TopoDS_Shape DxfReader::Internal::createShapeSplineFit(const Dxf_POLYLINE& polyline)
{
    const bool isPolylineClosed = polyline.flags & Dxf_POLYLINE::Flag::Closed;
    const gp_Dir extrusionDir = toOccDir(polyline.extrusionDirection);
    const Frame frame = makeOcsFrame(extrusionDir);

    std::vector<const Dxf_POLYLINE::Vertex*> ctrlPoints;
    std::vector<const Dxf_POLYLINE::Vertex*> splineVertices;
    std::vector<const Dxf_POLYLINE::Vertex*> allVertices;
    for (const Dxf_POLYLINE::Vertex& vertex : polyline.vertices) {
        allVertices.push_back(&vertex);
        if ((vertex.flags & Dxf_POLYLINE::Vertex::SplineFrameControlPoint) != 0)
            ctrlPoints.push_back(&vertex);
        else if ((vertex.flags & Dxf_POLYLINE::Vertex::SplineVertex) != 0)
            splineVertices.push_back(&vertex);
    }

    const std::vector<const Dxf_POLYLINE::Vertex*>& vertices =
        ctrlPoints.size() >= 2 ?
            ctrlPoints
            : (splineVertices.size() >= 2 ? splineVertices : allVertices)
        ;

    const int pointCount = int(vertices.size()) + (isPolylineClosed ? 1 : 0);
    auto points = makeOccHandle<TColgp_HArray1OfPnt>(1, pointCount);
    for (unsigned i = 0; i < vertices.size(); ++i)
        points->SetValue(i+1, ocsPointToWcs(vertices.at(i)->point, frame));

    if (isPolylineClosed)
        points->SetValue(pointCount, ocsPointToWcs(vertices.at(0)->point, frame));

    // BSpline interpolation
    GeomAPI_Interpolate interp(points, false/*!periodic*/, Precision::Confusion());
    interp.Perform();
    if (!interp.IsDone())
        return {}; // TODO Emit error

    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(interp.Curve());
    return makeExtrusionShape(edge, polyline.thickness, extrusionDir);
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_POLYLINE& polyline)
{
    if (polyline.flags & Dxf_POLYLINE::Flag::PolyfaceMesh) {
        return this->createShapePolyfaceMesh(polyline);
    }
    else if (polyline.flags & Dxf_POLYLINE::Flag::PolygonMesh3d) {
        return this->createShapePolygonMesh3d(polyline);
    }
    else if (polyline.flags & Dxf_POLYLINE::Flag::Polyline3d) {
        return this->createShapePolyline3d(polyline);
    }
    else if ((polyline.flags & Dxf_POLYLINE::Flag::CurveFit) != 0) {
        return this->createShapeCurveFit(polyline);
    }
    else if ((polyline.flags & Dxf_POLYLINE::Flag::SplineFit) != 0) {
        return this->createShapeSplineFit(polyline);
    }
    else {
        return this->createShapePolyline2d(polyline);
    }
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_SOLID& solid)
{
    Dxf_QuadBase quad = solid;
    if (solid.hasCorner4) {
        // See https://ezdxf.readthedocs.io/en/stable/dxfentities/solid.html
        std::swap(quad.corner3, quad.corner4);
    }

    try {
        return makeFace(quad);
    }
    catch (...) {
        m_messenger->emitError("createShape(Dxf_SOLID) failed");
        return {};
    }
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_SPLINE& spline)
{
    // https://documentation.help/AutoCAD-DXF/WS1a9193826455f5ff18cb41610ec0a2e719-79e1.htm
    try {
        if ((spline.flags & Dxf_SPLINE::Flag::Linear) != 0) {
            // TODO Build polyline
#if 0
            MeshUtils::Polygon3dBuilder polygonBuilder(nodeCount);
            for (unsigned i = 0; i < vertices.size(); ++i)
                polygonBuilder.setNode(i + 1, ocsPointToWcs(vertices.at(i).point, frame));

            if (isPolylineClosed)
                polygonBuilder.setNode(nodeCount, ocsPointToWcs(vertices.at(0).point, frame));

            polygonBuilder.finalize();
            const TopoDS_Shape shape = BRepUtils::makeEdge(polygonBuilder.get());
#endif
        }

        const bool hasExactNurbs =
            spline.degree > 0
            && !spline.controlPoints.empty()
            && !spline.knots.empty()
        ;
        if (hasExactNurbs)
            return createSplineFromPolesAndKnots(spline);
        else if (!spline.fitPoints.empty())
            return createInterpolationSpline(spline);
    }
    catch (const Standard_Failure& err) {
        m_messenger->emitWarning(
            fmt::format("DxfReader - Failed to create bspline({})", err.GetMessageString())
        );
    }

    return {};
}

TopoDS_Shape DxfReader::Internal::createShape(const Dxf_TEXT& text)
{
    if (!m_params.importAnnotations)
        return {};

    const Dxf_STYLE* ptrStyle = this->findStyle(text.styleName);
    std::string fontName = ptrStyle ? std::string{ptrStyle->name} : m_params.fontNameForTextObjects;
    // "ARIAL_NARROW" -> "ARIAL NARROW"
    if (toLowerCase_C(fontName) == "arial_narrow")
        fontName.replace(5, 1, " ");

    const double fontHeight = 1.4 * text.height * m_params.scaling;
    Font_BRepFont brepFont;
    brepFont.SetWidthScaling(static_cast<float>(text.relativeXScaleFactorWidth));
    if (!brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight/*, Font_StrictLevel_Aliases*/)) {
        m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        return {};
    }

    using DxfHJustification = Dxf_TEXT::HorizontalJustification;
    using DxfVJustification = Dxf_TEXT::VerticalJustification;
    // TEXT justification is subtle(eg baseline and fit modes)
    // See doc https://ezdxf.readthedocs.io/en/stable/tutorials/text.html
    const DxfHJustification hjust = text.horizontalJustification;
    Graphic3d_HorizontalTextAlignment hAlign = Graphic3d_HTA_LEFT;
    if (hjust == DxfHJustification::Center || hjust == DxfHJustification::Middle)
        hAlign = Graphic3d_HTA_CENTER;
    else if (hjust == DxfHJustification::Right)
        hAlign = Graphic3d_HTA_RIGHT;

    const DxfVJustification vjust = text.verticalJustification;
    Graphic3d_VerticalTextAlignment vAlign = Graphic3d_VTA_TOP;
    if (vjust == DxfVJustification::Baseline)
        vAlign = Graphic3d_VTA_TOPFIRSTLINE;
    else if (vjust == DxfVJustification::Bottom)
        vAlign = Graphic3d_VTA_BOTTOM;
    else if (vjust == DxfVJustification::Middle)
        vAlign = Graphic3d_VTA_CENTER;

    // Alignment point
    const bool applyFirstAlignPnt =
        hjust == DxfHJustification::Left
        || vjust == DxfVJustification::Baseline
    ;

    const DxfCoords& alignPnt = applyFirstAlignPnt ? text.firstAlignmentPoint : text.secondAlignmentPoint;
    const gp_Pnt pt = this->toPnt(alignPnt);

    gp_Vec xAxisDir = gp::DX();
    if (hjust == DxfHJustification::Aligned || hjust == DxfHJustification::Fit) {
        const gp_Pnt p1 = this->toPnt(text.firstAlignmentPoint);
        const gp_Pnt p2 = this->toPnt(text.secondAlignmentPoint);
        xAxisDir = gp_Vec{p1, p2};

        // Ensure non-null x-axis direction
        if (xAxisDir.Magnitude() < gp::Resolution())
            xAxisDir = gp::DX();
    }

    // If rotation angle is non-null and x-axis direction defaults to standard Ox then set x-axis
    // so it matches rotation angle
    xAxisDir.Normalize();
    if (!MathUtils::fuzzyIsNull(text.rotationAngle)
        && xAxisDir.IsEqual(gp::DX(), Precision::Confusion(), Precision::Angular()))
    {
        const double angle = MathUtils::degreeToRadian(text.rotationAngle);
        const gp_Trsf trsf = GeomUtils::makeRotation(gp_Ax1(pt, gp::DZ()), angle);
        xAxisDir = gp::DX().Transformed(trsf);
    }

    const gp_Dir extDir = toOccDir(text.extrusionDirection);
    const gp_Ax3 locText(pt, extDir, xAxisDir);
    Font_BRepTextBuilder brepTextBuilder;
    const auto occTextStr = string_conv<NCollection_String>(text.str);
    return brepTextBuilder.Perform(brepFont, occTextStr, locText, hAlign, vAlign);
}

void DxfReader::Internal::ReportError(const std::string& msg)
{
    m_messenger->emitError(msg);
}

gp_Pnt DxfReader::Internal::toPnt(const DxfCoords& coords) const
{
    double sp1 = coords.x;
    double sp2 = coords.y;
    double sp3 = coords.z;
    if (!MathUtils::fuzzyEqual(m_params.scaling, 1.)) {
        sp1 = sp1 * m_params.scaling;
        sp2 = sp2 * m_params.scaling;
        sp3 = sp3 * m_params.scaling;
    }

    return gp_Pnt{sp1, sp2, sp3};
}

void DxfReader::Internal::addShape(const TopoDS_Shape& shape, const Dxf_BaseEntity& srcEntity)
{
#if 0
    const Dxf_LAYER* layer = m_internal->findLayer(srcEntity.layerName);
    DxfColorIndex colorId = srcEntity.colorId;
    if (colorId == dxfColorByLayer && layer)
        colorId = layer->colorId;

    TDF_Label layerLabel;
    TopoDS_Shape layerShape;
    if (layer) {
        layerLabel = Cpp::findValue(srcEntity.layerName, m_mapOccLayerLabel);
        if (layerLabel.IsNull())
            layerLabel = m_targetDoc->xcaf().layerTool()->AddLayer(to_OccExtString(srcEntity.layerName));

        layerShape = Cpp::findValue(srcEntity.layerName, m_mapOccLayerShape);
        if (layerShape.IsNull())
            layerShape = BRepUtils::makeEmptyCompound();
    }

    if (layerShape) {
        BRepUtils::addShape(&layerShape, shape);
    }

    // TODO Handle color

    if (layerLabel)
        m_targetDoc->xcaf().layerTool()->SetLayer();

    // BRepUtils::addShape(&comp, entity.shape);

    const Entity newEntity{ m_ColorIndex, shape };
    const std::string layerName = this->LayerName();
    auto itFound = m_layers.find(layerName);
    if (itFound != m_layers.end()) {
        std::vector<DxfReader::Entity>& vecEntity = itFound->second;
        vecEntity.push_back(newEntity);
    }
    else {
        decltype(m_layers)::value_type pair(std::move(layerName), { newEntity });
        m_layers.insert(std::move(pair));
    }
#endif
}

TopoDS_Face DxfReader::Internal::makeFace(const Dxf_QuadBase& quad)
{
    const gp_Pnt p1 = toOccPnt(quad.corner1);
    const gp_Pnt p2 = toOccPnt(quad.corner2);
    const gp_Pnt p3 = toOccPnt(quad.corner3);
    const gp_Pnt p4 = toOccPnt(quad.corner4);

    const double pntTolerance = Precision::Confusion();
    if (p1.IsEqual(p2, pntTolerance) || p1.IsEqual(p3, pntTolerance) || p2.IsEqual(p3, pntTolerance))
        return {};

    TopoDS_Face face;
    BRepBuilderAPI_MakeWire makeWire;
    makeWire.Add(BRepBuilderAPI_MakeEdge(p1, p2));
    makeWire.Add(BRepBuilderAPI_MakeEdge(p2, p3));
    if (quad.hasCorner4 && !p3.IsEqual(p4, pntTolerance) && !p1.IsEqual(p4, pntTolerance)) {
        makeWire.Add(BRepBuilderAPI_MakeEdge(p3, p4));
        makeWire.Add(BRepBuilderAPI_MakeEdge(p4, p1));
    }
    else {
        makeWire.Add(BRepBuilderAPI_MakeEdge(p3, p1));
    }

    if (makeWire.IsDone())
        face = BRepBuilderAPI_MakeFace(makeWire.Wire(), true/*onlyPlane*/);

    return face;
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
TopoDS_Shape DxfReader::Internal::createSplineFromPolesAndKnots(const Dxf_SPLINE& spline)
{
    if (spline.weights.size() > spline.controlPoints.size())
        return {};

    const bool isPeriodic = (spline.flags & Dxf_SPLINE::Periodic) != 0;
    const bool isRational = (spline.flags & Dxf_SPLINE::Rational) != 0;

    // Handle poles
    const auto iNumPoles = gsl::narrow<int>(spline.controlPoints.size());
    TColgp_Array1OfPnt occPoles(1, iNumPoles);
    for (const DxfCoords& pnt : spline.controlPoints) {
        const auto iPnt = static_cast<int>(&pnt - &spline.controlPoints.front());
        occPoles.ChangeValue(iPnt + 1) = toOccPnt(pnt);
    }

    // Handle knots and mults
    const auto iNumKnots = gsl::narrow<int>(spline.knots.size());
    TColStd_Array1OfReal occKnots(1, iNumKnots);
    std::copy(spline.knots.cbegin(), spline.knots.cend(), occKnots.begin());

    const auto iNumUniqueKnots = BSplCLib::KnotsLength(occKnots, isPeriodic);
    TColStd_Array1OfReal occUniqueKnots(1, iNumUniqueKnots);
    TColStd_Array1OfInteger occMults(1, iNumUniqueKnots);
    BSplCLib::Knots(occKnots, std::ref(occUniqueKnots), std::ref(occMults), isPeriodic);

    // Handle weights
    TColStd_Array1OfReal occWeights(1, iNumPoles);
    if (isRational && spline.weights.size() == spline.controlPoints.size())
        std::copy(spline.weights.cbegin(), spline.weights.cend(), occWeights.begin());
    else
        std::fill(occWeights.begin(), occWeights.end(), 1.); // Non-rational

#ifdef MAYO_IO_DXF_DEBUG_TRACE
    // Debug traces
    std::cout << std::endl << "createSplineFromPolesAndKnots()";
    std::cout << "\n    degree: " << spline.degree;
    std::cout << "\n    flags: " << spline.flags;
    std::cout << "\n    isPeriodic: " << isPeriodic;
    std::cout << "\n    numPoles: " << iNumPoles;

    std::cout << "\n    numKnots: " << iNumKnots;
    std::cout << "\n    occKnots: ";
    for (double uknot : occKnots)
        std::cout << uknot << ", ";

    std::cout << "\n    numUniqueKnots: " << iNumUniqueKnots;
    std::cout << "\n    occUniqueKnots: ";
    for (double uknot : occUniqueKnots)
        std::cout << uknot << ", ";

    std::cout << "\n    occMults: ";
    for (int mult : occMults)
        std::cout << mult << ", ";

    std::cout << "\n    BSplCLib::NbPoles(): " << BSplCLib::NbPoles(spline.degree, isPeriodic, occMults);
    std::cout << std::endl;
#endif

    // Check internal mutls(forbidden: mult == degree+1 inside)
    bool hasForbiddenInternal = false;
    for (int i = (occMults.Lower() + 1); i < occMults.Upper(); ++i) {
        if (occMults[i] > spline.degree) { // required: <= degree
            hasForbiddenInternal = true;
            break;
        }
    }

    if (!hasForbiddenInternal) {
        auto curve = makeOccHandle<Geom_BSplineCurve>(
            occPoles, occWeights, occUniqueKnots, occMults, spline.degree, isPeriodic
        );
        if (curve)
            return BRepBuilderAPI_MakeEdge(curve);
    }
    else {
        // Split curve into cubic Bézier segments
        const int numPolesPerSegment = spline.degree + 1;
        const int numSegments = occUniqueKnots.Size() - 1;
        if (numSegments * numPolesPerSegment != iNumPoles)
            return {};

        BRepBuilderAPI_MakeWire wireBuilder;
        for (int i = 0; i < numSegments; ++i) {
            TColgp_Array1OfPnt segPoles(1, numPolesPerSegment);
            for (int j = 0; j < numPolesPerSegment; ++j)
                segPoles.SetValue(j+1, occPoles.Value(i * numPolesPerSegment + j + 1));

            wireBuilder.Add(BRepBuilderAPI_MakeEdge(makeOccHandle<Geom_BezierCurve>(segPoles)));
        }

        if (wireBuilder.IsDone())
            return wireBuilder.Wire();
    }

    return {};
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
TopoDS_Shape DxfReader::Internal::createInterpolationSpline(const Dxf_SPLINE& spline)
{
    const auto fitPointCount = gsl::narrow<int>(spline.fitPoints.size());

    // NOTE
    // GeomAPI_Interpolate wants an array of parameters(N+1) for periodic splines, but the DXF format
    // doesn't supply this. The solution is to set !isPeriodic for GeomAPI_Interpolate() and duplicate
    // first point at the end to close the curve(DXF closed/periodic)

    const bool isPeriodic = (spline.flags & Dxf_SPLINE::Periodic) != 0;
    const bool isClosed = (spline.flags & Dxf_SPLINE::Closed) != 0;
    const int addFirstPoint = isPeriodic || isClosed ? 1 : 0;

    // Handle poles
    auto fitpoints = makeOccHandle<TColgp_HArray1OfPnt>(1, fitPointCount + addFirstPoint);
    for (const DxfCoords& pnt : spline.fitPoints) {
        const auto iPnt = static_cast<int>(&pnt - &spline.fitPoints.front());
        fitpoints->ChangeValue(iPnt + 1) = toOccPnt(pnt);
    }

    if (addFirstPoint == 1)
        fitpoints->ChangeValue(fitpoints->Upper()) = toOccPnt(spline.fitPoints.front());

    GeomAPI_Interpolate interp(fitpoints, false/*isPeriodic*/, Precision::Confusion());
    const gp_Vec tanStart = toOccVec(spline.startTangent);
    const gp_Vec tanEnd = toOccVec(spline.endTangent);
    const bool hasTanStart = tanStart.SquareMagnitude() > Precision::SquareConfusion();
    const bool hasTanEnd = tanEnd.SquareMagnitude() > Precision::SquareConfusion();
    if (hasTanStart || hasTanEnd)
        interp.Load(tanStart, tanEnd, true/*scale*/);

    interp.Perform();
    if (!interp.IsDone())
        return {}; // TODO Emit error message

    return BRepBuilderAPI_MakeEdge(interp.Curve());
}

} // namespace Mayo::IO
