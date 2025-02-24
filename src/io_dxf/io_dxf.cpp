/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_dxf.h"

#include "../base/brep_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/filepath.h"
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
#include <BRep_Builder.hxx>
#include <BSplCLib.hxx>
#include <Font_BRepTextBuilder.hxx>
#include <Font_FontMgr.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <Graphic3d_VerticalTextAlignment.hxx>
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
#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

#define MAYO_IO_DXF_DEBUG_TRACE 1

namespace Mayo::IO {

namespace {

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

gp_Vec toOccVec(const DxfCoords& coords)
{
    return { coords.x, coords.y, coords.z };
}

} // namespace

class DxfReader::Internal : public CDxfRead {
private:
    Messenger* m_messenger = nullptr;
    DxfReader::Parameters m_params;
    std::unordered_map<std::string, std::vector<DxfReader::Entity>> m_layers;
    TaskProgress* m_progress = nullptr;
    std::uintmax_t m_fileSize = 0;
    std::uintmax_t m_fileReadSize = 0;
    Resource_FormatType m_srcEncoding = Resource_ANSI;

protected:
    void get_line() override;
    bool setSourceEncoding(const std::string& codepage) override;
    std::string toUtf8(const std::string& strSource) override;

public:
    Internal(const FilePath& filepath, TaskProgress* progress = nullptr);

    void setMessenger(Messenger* messenger) { m_messenger = messenger; }
    void setParameters(const DxfReader::Parameters& params) { m_params = params; }
    const auto& layers() const { return m_layers; }

    // CDxfRead's virtual functions
    void OnReadLine(const DxfCoords& s, const DxfCoords& e, bool hidden) override;
    void OnReadPolyline(const Dxf_POLYLINE& polyline) override;
    void OnReadPoint(const DxfCoords& s) override;
    void OnReadText(const Dxf_TEXT& text) override;
    void OnReadMText(const Dxf_MTEXT& text) override;
    void OnReadArc(const DxfCoords& s, const DxfCoords& e, const DxfCoords& c, bool dir, bool hidden) override;
    void OnReadCircle(const DxfCoords& s, const DxfCoords& c, bool dir, bool hidden) override;
    void OnReadEllipse(const DxfCoords& c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir) override;
    void OnReadSpline(const Dxf_SPLINE& spline) override;
    void OnReadInsert(const Dxf_INSERT& ins) override;
    void OnReadDimension(const DxfCoords& s, const DxfCoords& e, const DxfCoords& point, double rotation) override;
    void OnReadSolid(const Dxf_SOLID& solid) override;
    void OnRead3dFace(const Dxf_3DFACE& face) override;

    void ReportError(const std::string& msg) override;
    void AddGraphics() const override;

    static OccHandle<Geom_BSplineCurve> createSplineFromPolesAndKnots(const Dxf_SPLINE& spline);
    static OccHandle<Geom_BSplineCurve> createInterpolationSpline(const Dxf_SPLINE& spline);

    gp_Pnt toPnt(const DxfCoords& coords) const;
    void addShape(const TopoDS_Shape& shape);

    TopoDS_Face makeFace(const Dxf_QuadBase& quad) const;
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

bool DxfReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    m_layers.clear();
    DxfReader::Internal internalReader(filepath, progress);
    internalReader.setParameters(m_params);
    internalReader.setMessenger(this->messenger() ? this->messenger() : &Messenger::null());
    internalReader.DoRead();
    m_layers = std::move(internalReader.layers());
    return !internalReader.Failed();
}

TDF_LabelSequence DxfReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    TDF_LabelSequence seqLabel;
    OccHandle<XCAFDoc_ShapeTool> shapeTool = doc->xcaf().shapeTool();
    OccHandle<XCAFDoc_ColorTool> colorTool = doc->xcaf().colorTool();
    OccHandle<XCAFDoc_LayerTool> layerTool = doc->xcaf().layerTool();
    std::unordered_map<std::string, TDF_Label> mapLayerNameLabel;
    std::unordered_map<ColorIndex_t, TDF_Label> mapAciColorLabel;

    auto fnAddRootShape = [&](const TopoDS_Shape& shape, const std::string& shapeName, TDF_Label layer) {
        const TDF_Label labelShape = shapeTool->NewShape();
        shapeTool->SetShape(labelShape, shape);
        TDataStd_Name::Set(labelShape, to_OccExtString(shapeName));
        seqLabel.Append(labelShape);
        if (!layer.IsNull())
            layerTool->SetLayer(labelShape, layer, true/*onlyInOneLayer*/);

        return labelShape;
    };

    auto fnAddAci = [&](ColorIndex_t aci) -> TDF_Label {
        auto it = mapAciColorLabel.find(aci);
        if (it != mapAciColorLabel.cend())
            return it->second;

        if (0 <= aci && CppUtils::cmpLess(aci, std::size(aciTable))) {
            const RGB_Color& c = aciTable[aci].second;
            const TDF_Label colorLabel = colorTool->AddColor(
                        Quantity_Color(c.r / 255., c.g / 255., c.b / 255., Quantity_TOC_RGB)
            );
            mapAciColorLabel.insert({ aci, colorLabel });
            return colorLabel;
        }

        return TDF_Label();
    };

    int iShape = 0;
    int shapeCount = 0;
    for (const auto& [layerName, vecEntity] : m_layers) {
        if (!startsWith(layerName, "BLOCKS")) {
            shapeCount = CppUtils::safeStaticCast<int>(shapeCount + vecEntity.size());
            const TDF_Label layerLabel = layerTool->AddLayer(to_OccExtString(layerName));
            mapLayerNameLabel.insert({ layerName, layerLabel });
        }
    }
    auto fnUpdateProgressValue = [&]{
        progress->setValue(MathUtils::toPercent(iShape, 0, shapeCount));
    };

    auto fnSetShapeColor = [=](const TDF_Label& labelShape, int aci) {
        const TDF_Label labelColor = fnAddAci(aci);
        if (!labelColor.IsNull())
            colorTool->SetColor(labelShape, labelColor, XCAFDoc_ColorGen);
    };

    if (!m_params.groupLayers) {
        for (const auto& [layerName, vecEntity] : m_layers) {
            if (startsWith(layerName, "BLOCKS"))
                continue; // Skip

            const TDF_Label layerLabel = CppUtils::findValue(layerName, mapLayerNameLabel);
            for (const DxfReader::Entity& entity : vecEntity) {
                const std::string shapeName = std::string("Shape_") + std::to_string(++iShape);
                const TDF_Label shapeLabel = fnAddRootShape(entity.shape, shapeName, layerLabel);
                colorTool->SetColor(shapeLabel, fnAddAci(entity.aci), XCAFDoc_ColorGen);
                fnUpdateProgressValue();
            }
        }
    }
    else {
        for (const auto& [layerName, vecEntity] : m_layers) {
            if (startsWith(layerName, "BLOCKS"))
                continue; // Skip

            TopoDS_Compound comp = BRepUtils::makeEmptyCompound();
            for (const Entity& entity : vecEntity) {
                if (!entity.shape.IsNull())
                    BRepUtils::addShape(&comp, entity.shape);
            }

            if (!comp.IsNull()) {
                const TDF_Label layerLabel = CppUtils::findValue(layerName, mapLayerNameLabel);
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

            iShape = CppUtils::safeStaticCast<int>(iShape + vecEntity.size());
            fnUpdateProgressValue();
        }
    }

    return seqLabel;
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

void DxfReader::Internal::get_line()
{
    CDxfRead::get_line();
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

DxfReader::Internal::Internal(const FilePath& filepath, TaskProgress* progress)
    : CDxfRead(filepath.u8string().c_str()),
      m_progress(progress)
{
    if (!this->Failed())
        m_fileSize = filepathFileSize(filepath);
}

void DxfReader::Internal::OnReadLine(const DxfCoords& s, const DxfCoords& e, bool /*hidden*/)
{
    const gp_Pnt p0 = this->toPnt(s);
    const gp_Pnt p1 = this->toPnt(e);
    if (p0.IsEqual(p1, Precision::Confusion()))
        return;

    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p0, p1);
    this->addShape(edge);
}

void DxfReader::Internal::OnReadPolyline(const Dxf_POLYLINE& polyline)
{
    const auto& vertices = polyline.vertices;
    if (polyline.flags & Dxf_POLYLINE::Flag::PolyfaceMesh) {
        const int meshVertexCount = polyline.polygonMeshMVertexCount;
        TColgp_Array1OfPnt nodes(1, meshVertexCount);
        for (int i = 0; i < meshVertexCount; ++i)
            nodes.ChangeValue(i + 1) = this->toPnt(vertices.at(i).point);

        const int meshFaceCount = polyline.polygonMeshNVertexCount;
        std::vector<Poly_Triangle> vecTriangle;
        vecTriangle.reserve(meshFaceCount);
        for (int i = 0; i < meshFaceCount; ++i) {
            const Dxf_VERTEX& face = vertices.at(meshVertexCount + i);
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

        this->addShape(BRepUtils::makeFace(new Poly_Triangulation(nodes, triangles)));
    }
    else {
        const bool isPolylineClosed = polyline.flags & Dxf_POLYLINE::Flag::Closed;
        const int nodeCount = CppUtils::safeStaticCast<int>(vertices.size() + (isPolylineClosed ? 1 : 0));
        MeshUtils::Polygon3dBuilder polygonBuilder(nodeCount);
        for (unsigned i = 0; i < vertices.size(); ++i)
            polygonBuilder.setNode(i + 1, this->toPnt(vertices.at(i).point));

        if (isPolylineClosed)
            polygonBuilder.setNode(nodeCount, this->toPnt(vertices.at(0).point));

        polygonBuilder.finalize();
        this->addShape(BRepUtils::makeEdge(polygonBuilder.get()));
    }
}

void DxfReader::Internal::OnReadPoint(const DxfCoords& s)
{
    const TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(this->toPnt(s));
    this->addShape(vertex);
}

void DxfReader::Internal::OnReadText(const Dxf_TEXT& text)
{
    if (!m_params.importAnnotations)
        return;

    const std::string layerName = this->LayerName();
    if (startsWith(layerName, "BLOCKS"))
        return;

    const Dxf_STYLE* ptrStyle = this->findStyle(text.styleName);
    std::string fontName = ptrStyle ? ptrStyle->name : m_params.fontNameForTextObjects;
    // "ARIAL_NARROW" -> "ARIAL NARROW"
    if (toLowerCase_C(fontName) == "arial_narrow")
        fontName.replace(5, 1, " ");

    const double fontHeight = 1.4 * text.height * m_params.scaling;
    Font_BRepFont brepFont;
    brepFont.SetWidthScaling(static_cast<float>(text.relativeXScaleFactorWidth));
    if (!brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight/*, Font_StrictLevel_Aliases*/)) {
        m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        return;
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

    // Ensure non-null extrusion direction
    gp_Vec extDir = toOccVec(text.extrusionDirection);
    if (extDir.Magnitude() < gp::Resolution())
        extDir = gp::DZ();

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
        gp_Trsf rotTrsf;
        rotTrsf.SetRotation(gp_Ax1(pt, gp::DZ()), text.rotationAngle);
        xAxisDir = gp::DX().Transformed(rotTrsf);
    }

    const gp_Ax3 locText(pt, extDir, xAxisDir);
    Font_BRepTextBuilder brepTextBuilder;
    const auto occTextStr = string_conv<NCollection_String>(text.str);
    const TopoDS_Shape shapeText = brepTextBuilder.Perform(brepFont, occTextStr, locText, hAlign, vAlign);
    this->addShape(shapeText);
}

void DxfReader::Internal::OnReadMText(const Dxf_MTEXT& text)
{
    if (!m_params.importAnnotations)
        return;

    const gp_Pnt pt = this->toPnt(text.insertionPoint);
    const std::string layerName = this->LayerName();
    if (startsWith(layerName, "BLOCKS"))
        return;

    const std::string& fontName = m_params.fontNameForTextObjects;
    const double fontHeight = 1.4 * text.height * m_params.scaling;
    Font_BRepFont brepFont;
    if (!brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight)) {
        m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        return;
    }

    const int ap = static_cast<int>(text.attachmentPoint);
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

    // Ensure non-null extrusion direction
    gp_Vec extDir = toOccVec(text.extrusionDirection);
    if (extDir.Magnitude() < gp::Resolution())
        extDir = gp::DZ();

    // Ensure non-null x-axis direction
    gp_Vec xAxisDir = toOccVec(text.xAxisDirection);
    if (xAxisDir.Magnitude() < gp::Resolution())
        xAxisDir = gp::DX();

    // If rotation angle is non-null and x-axis direction defaults to standard Ox then set x-axis
    // so it matches rotation angle
    xAxisDir.Normalize();
    if (!MathUtils::fuzzyIsNull(text.rotationAngle)
        && xAxisDir.IsEqual(gp::DX(), Precision::Confusion(), Precision::Angular()))
    {
        gp_Trsf rotTrsf;
        rotTrsf.SetRotation(gp_Ax1(pt, gp::DZ()), text.rotationAngle);
        xAxisDir = gp::DX().Transformed(rotTrsf);
    }

    const auto occTextStr = string_conv<NCollection_String>(text.str);
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
    const TopoDS_Shape shapeText = brepTextBuilder.Perform(brepFont, textFormat, locText);
#else
    const TopoDS_Shape shapeText = brepTextBuilder.Perform(brepFont, occTextStr, locText, hAlign, vAlign);
#endif

    this->addShape(shapeText);
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadArc(const DxfCoords& s, const DxfCoords& e, const DxfCoords& c, bool dir, bool /*hidden*/)
{
    const gp_Pnt p0 = this->toPnt(s);
    const gp_Pnt p1 = this->toPnt(e);
    const gp_Dir up = dir ? gp::DZ() : -gp::DZ();
    const gp_Pnt pc = this->toPnt(c);
    const gp_Circ circle(gp_Ax2(pc, up), p0.Distance(pc));
    if (circle.Radius() > 0) {
        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circle, p0, p1);
        this->addShape(edge);
    }
    else {
        m_messenger->emitWarning("DxfReader - Ignore degenerate arc of circle");
    }
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadCircle(const DxfCoords& s, const DxfCoords& c, bool dir, bool /*hidden*/)
{
    const gp_Pnt p0 = this->toPnt(s);
    const gp_Dir up = dir ? gp::DZ() : -gp::DZ();
    const gp_Pnt pc = this->toPnt(c);
    const gp_Circ circle(gp_Ax2(pc, up), p0.Distance(pc));
    if (circle.Radius() > 0) {
        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circle);
        this->addShape(edge);
    }
    else {
        m_messenger->emitWarning("DxfReader - Ignore degenerate circle");
    }
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadEllipse(
        const DxfCoords& c,
        double major_radius, double minor_radius,
        double rotation,
        double /*start_angle*/, double /*end_angle*/,
        bool dir
    )
{
    const gp_Dir up = dir ? gp::DZ() : -gp::DZ();
    const gp_Pnt pc = this->toPnt(c);
    gp_Elips ellipse(
                gp_Ax2(pc, up),
                major_radius * m_params.scaling,
                minor_radius * m_params.scaling);
    ellipse.Rotate(gp_Ax1(pc, up), rotation);
    if (ellipse.MinorRadius() > 0) {
        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(ellipse);
        this->addShape(edge);
    }
    else {
        m_messenger->emitWarning("DxfReader - Ignore degenerate ellipse");
    }
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadSpline(const Dxf_SPLINE& spline)
{
    // https://documentation.help/AutoCAD-DXF/WS1a9193826455f5ff18cb41610ec0a2e719-79e1.htm
    try {
        OccHandle<Geom_BSplineCurve> geom;
        if (!spline.controlPoints.empty())
            geom = createSplineFromPolesAndKnots(spline);
        else if (!spline.fitPoints.empty())
            geom = createInterpolationSpline(spline);

        if (geom.IsNull())
            throw Standard_Failure("Geom_BSplineCurve object is null");

        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(geom);
        this->addShape(edge);
    }
    catch (const Standard_Failure& err) {
#ifdef MAYO_IO_DXF_DEBUG_TRACE
        std::cout << "ERROR DxfReader::OnReadSpline() -- " << err.GetMessageString() << std::endl;
#endif
        m_messenger->emitWarning(
            fmt::format("DxfReader - Failed to create bspline({})", err.GetMessageString())
        );
    }
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadInsert(const Dxf_INSERT& ins)
{
    const std::string prefix = "BLOCKS " + ins.blockName + " ";
    for (const auto& [k, vecEntity] : m_layers) {
        if (!startsWith(k, prefix))
            continue; // Skip

        TopoDS_Shape comp = BRepUtils::makeEmptyCompound();
        for (const DxfReader::Entity& entity : vecEntity) {
            if (!entity.shape.IsNull())
                BRepUtils::addShape(&comp, entity.shape);
        }

        if (comp.IsNull())
            continue; // Skip

        if (!MathUtils::fuzzyEqual(ins.scaleFactor.x, ins.scaleFactor.y)
            || !MathUtils::fuzzyEqual(ins.scaleFactor.x, ins.scaleFactor.y)
           )
        {
            m_messenger->emitWarning(
                fmt::format("OnReadInsert('{}') - non-uniform scales aren't supported({}, {}, {})",
                            ins.blockName, ins.scaleFactor.x, ins.scaleFactor.y, ins.scaleFactor.z
                )
            );
        }

        auto fnNonNull = [](double v) { return !MathUtils::fuzzyIsNull(v) ? v : 1.; };
        const double avgScale = std::abs(fnNonNull((ins.scaleFactor.x + ins.scaleFactor.y + ins.scaleFactor.z) / 3.));
        if (!MathUtils::fuzzyEqual(avgScale, 1.)) {
            gp_Trsf trsf;
            trsf.SetScaleFactor(avgScale);
            BRepBuilderAPI_Transform brepTrsf(comp, trsf);
            if (brepTrsf.IsDone()) {
                comp = brepTrsf.Shape();
            }
            else {
                m_messenger->emitWarning(
                    fmt::format("OnReadInsert('{}') - scaling failed({}, {}, {})",
                                ins.blockName, ins.scaleFactor.x, ins.scaleFactor.y, ins.scaleFactor.z
                    )
                );
            }
        }

        gp_Trsf trsfRotZ;
        if (!MathUtils::fuzzyIsNull(ins.rotationAngle))
            trsfRotZ.SetRotation(gp::OZ(), ins.rotationAngle);

        gp_Trsf trsfMove;
        trsfMove.SetTranslation(this->toPnt(ins.insertPoint).XYZ());

        comp.Location(trsfRotZ * trsfMove);
        this->addShape(comp);
    }
}

void DxfReader::Internal::OnReadDimension(const DxfCoords& s, const DxfCoords& e, const DxfCoords& point, double rotation)
{
    if (m_params.importAnnotations) {
        // TODO
        std::stringstream sstr;
        sstr << "DxfReader::OnReadDimension() - Not yet implemented" << std::endl
             << "    s: " << s.x << ", " << s.y << ", " << s.z << std::endl
             << "    e: " << e.x << ", " << e.y << ", " << e.z << std::endl
             << "    point: " << point.x << ", " << point.y << ", " << point.z << std::endl
             << "    rotation: " << rotation << std::endl;
        m_messenger->emitWarning(sstr.str());
    }
}

void DxfReader::Internal::OnReadSolid(const Dxf_SOLID& solid)
{
    Dxf_QuadBase quad = solid;
    if (solid.hasCorner4) {
        // See https://ezdxf.readthedocs.io/en/stable/dxfentities/solid.html
        std::swap(quad.corner3, quad.corner4);
    }

    try {
        const TopoDS_Face face = makeFace(quad);
        if (!face.IsNull())
            this->addShape(face);
    }
    catch (...) {
        m_messenger->emitError("OnReadSolid() failed");
    }
}

void DxfReader::Internal::OnRead3dFace(const Dxf_3DFACE& face)
{
    try {
        const TopoDS_Face brepFace = makeFace(face);
        if (!brepFace.IsNull())
            this->addShape(brepFace);
    }
    catch (...) {
        m_messenger->emitError("OnReadFace() failed");
    }
}

void DxfReader::Internal::ReportError(const std::string& msg)
{
    m_messenger->emitError(msg);
}

void DxfReader::Internal::AddGraphics() const
{
    // Nothing
}

gp_Pnt DxfReader::Internal::toPnt(const DxfCoords& coords) const
{
    double sp1(coords.x);
    double sp2(coords.y);
    double sp3(coords.z);
    if (!MathUtils::fuzzyEqual(m_params.scaling, 1.)) {
        sp1 = sp1 * m_params.scaling;
        sp2 = sp2 * m_params.scaling;
        sp3 = sp3 * m_params.scaling;
    }

    return gp_Pnt(sp1, sp2, sp3);
}

void DxfReader::Internal::addShape(const TopoDS_Shape& shape)
{
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
}

TopoDS_Face DxfReader::Internal::makeFace(const Dxf_QuadBase& quad) const
{
    const gp_Pnt p1 = this->toPnt(quad.corner1);
    const gp_Pnt p2 = this->toPnt(quad.corner2);
    const gp_Pnt p3 = this->toPnt(quad.corner3);
    const gp_Pnt p4 = this->toPnt(quad.corner4);

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
OccHandle<Geom_BSplineCurve> DxfReader::Internal::createSplineFromPolesAndKnots(const Dxf_SPLINE& spline)
{
    if (spline.weights.size() > spline.controlPoints.size())
        return {};

    const bool isPeriodic = (spline.flags & Dxf_SPLINE::Periodic) != 0;

    // Handle poles
    const auto iNumPoles = CppUtils::safeStaticCast<int>(spline.controlPoints.size());
    TColgp_Array1OfPnt occPoles(1, iNumPoles);
    for (const DxfCoords& pnt : spline.controlPoints) {
        const auto iPnt = CppUtils::safeStaticCast<int>(&pnt - &spline.controlPoints.front());
        occPoles.ChangeValue(iPnt + 1) = gp_Pnt{pnt.x, pnt.y, pnt.z};
    }

    // Handle knots and mults
    const auto iNumKnots = CppUtils::safeStaticCast<int>(spline.knots.size());
    TColStd_Array1OfReal occKnots(1, iNumKnots);
    std::copy(spline.knots.cbegin(), spline.knots.cend(), occKnots.begin());

    const auto iNumUniqueKnots = BSplCLib::KnotsLength(occKnots, isPeriodic);
    TColStd_Array1OfReal occUniqueKnots(1, iNumUniqueKnots);
    TColStd_Array1OfInteger occMults(1, iNumUniqueKnots);
    BSplCLib::Knots(occKnots, std::ref(occUniqueKnots), std::ref(occMults), isPeriodic);

    // Handle weights
    TColStd_Array1OfReal occWeights(1, iNumPoles);
    if (spline.weights.size() == spline.controlPoints.size())
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

    std::cout << "\n    numStartTangents: " << spline.startTangents.size();
    std::cout << "\n    numEndTangents: " << spline.endTangents.size();
    std::cout << "\n    BSplCLib::NbPoles(): " << BSplCLib::NbPoles(spline.degree, isPeriodic, occMults);
    std::cout << std::endl;
#endif

    return new Geom_BSplineCurve(occPoles, occWeights, occUniqueKnots, occMults, spline.degree, isPeriodic);
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
OccHandle<Geom_BSplineCurve> DxfReader::Internal::createInterpolationSpline(const Dxf_SPLINE& spline)
{
    const auto iNumPoints = CppUtils::safeStaticCast<int>(spline.fitPoints.size());

    // Handle poles
    auto fitpoints = makeOccHandle<TColgp_HArray1OfPnt>(1, iNumPoints);
    for (const DxfCoords& pnt : spline.fitPoints) {
        const auto iPnt = CppUtils::safeStaticCast<int>(&pnt - &spline.fitPoints.front());
        fitpoints->ChangeValue(iPnt + 1) = gp_Pnt{pnt.x, pnt.y, pnt.z};
    }

    const bool isPeriodic = (spline.flags & Dxf_SPLINE::Periodic) != 0;
    GeomAPI_Interpolate interp(fitpoints, isPeriodic, Precision::Confusion());
    interp.Perform();
    return interp.Curve();
}

} // namespace Mayo::IO
