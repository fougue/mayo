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
#include "../base/libfromchars.h"
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
#include "dxf_parser.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRep_Builder.hxx>
#include <BSplCLib.hxx>
#include <ElCLib.hxx>
#include <Font_BRepTextBuilder.hxx>
#include <Font_FontMgr.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
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
#include <regex>
#include <string_view>

//#define MAYO_IO_DXF_DEBUG_TRACE 1

namespace Mayo::IO {

namespace {

template<typename T>
using ConstRefWrap = std::reference_wrapper<const T>;

// Is `ch` a digit character in "classic" locale?
bool isDigit_C(char ch)
{
    return '0' <= ch && ch <= '9';
}

// Returns `ch` converted to lower-case, assuming "classic" locale
char toLowerCase_C(char ch)
{
    return std::tolower(ch, std::locale::classic());
}

// Returns string `str` converted to lower-case, assuming "classic" locale
std::string toLowerCase_C(const std::string& str)
{
    std::string lstr = str;
    for (char& ch : lstr)
        ch = toLowerCase_C(ch);

    return lstr;
}

// Converts the Unicode code unit encoded in UTF‑16 into its corresponding UTF‑8 byte sequence
// It supports all Unicode scalar values within the Basic Multilingual Plane(BMP) — ie code points
// from U+0000 to U+FFFF, excluding surrogate values.
// The function returns a std::array<char, 4> containing the UTF‑8 bytes followed by a null terminator.
// This function is useful for interpreting Unicode escape sequences such as \U+nnnn found in DXF
// MTEXT which are always 16‑bit BMP code points
std::array<char, 4> toUtf8(char16_t code)
{
    // Buffer to hold the utf8 result(up to 3 bytes + terminating '\0').
    // Defaults to '?' if the code does not fall into any handled range
    std::array<char, 4> u8Code = {'?', '\0', '\0', '\0'};
    if (code <= 0x7F) {
        // Case 1: 0000..007F → ASCII encoded in one utf8 byte
        // utf8 layout for ASCII: 0xxxxxxx
        u8Code[0] = char(code);
    }
    else if (code <= 0x7FF) {
        // Case 2: 0080..07FF → encoded in two utf8 bytes
        // utf8 layout: 110xxxxx 10xxxxxx
        // 1st byte:
        //   - 0xC0 sets the leading bits to 110xxxxx
        //   - (code >> 6) places the upper 5 bits of the code point into 'xxxxx'
        u8Code[0] = char(0xC0 | (code >> 6));
        // 2nd byte:
        //   - 0x80 sets the leading bits to 10xxxxxx(continuation byte)
        //   - (code & 0x3F) places the lower 6 bits of the code point into 'xxxxxx'
        u8Code[1] = char(0x80 | (code & 0x3F));
    }
    else if (code <= 0xFFFF) {
        // Case 3: 0800..FFFF → encoded in three utf8 bytes
        // utf8 layout: 1110xxxx 10xxxxxx 10xxxxxx
        // 1st byte:
        //   - 0xE0 sets the leading bits to 1110xxxx
        //   - (code >> 12) puts the top 4 bits of the code point in 'xxxx'
        u8Code[0] = char(0xE0 | (code >> 12));
        // 2nd byte:
        //   - 0x80 marks a continuation byte(10xxxxxx)
        //   - ((code >> 6) & 0x3F) extracts the next 6 bits
        u8Code[1] = char(0x80 | ((code >> 6) & 0x3F));
        // 3rd byte:
        //   - 0x80 marks a continuation byte(10xxxxxx)
        //   - (code & 0x3F) provides the lowest 6 bits
        u8Code[2] = char(0x80 | (code & 0x3F));
    }

    return u8Code;
}

// Returns enumeration of all font names on the system
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

// Returns graphical width of string `str` using input BREP font
double computeStringWidth(const NCollection_String& str, Font_BRepFont& brepFont)
{
    double w = 0.;
    for (auto it = str.Iterator(); it.Index() < str.Length(); ) {
        const Standard_Utf32Char ch = *it;
        ++it;
        const Standard_Utf32Char chNext = (it.Index() < str.Length()) ? *it : 0;
        w += brepFont.AdvanceX(ch, chNext);
    }

    return w;
}

gp_Pnt toCorrectedPnt(const gp_Pnt& pnt)
{
    auto fixCoordValue = [](double coord, double fallback = 0.) {
        if (std::isnan(coord))
            return fallback;

        if (!std::isfinite(coord))
            return fallback;

        // Incoherent value, too big for OpenCascade
        if (std::abs(coord) > 1e50)
            return fallback;

        return coord;
    };

    return {fixCoordValue(pnt.X()), fixCoordValue(pnt.Y()), fixCoordValue(pnt.Z())};
}

gp_Pnt toOccPnt(const DxfCoords& coords)
{
    return toCorrectedPnt({ coords.x, coords.y, coords.z });
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
    if (GeomUtils::isNull(v))
        return defaultDir;

    return v;
}

struct Frame {
    gp_Dir u;
    gp_Dir v;
    gp_Dir w;

    // Returns the frame with its (u, v) axes rotated by `theta` around `frame.w` (w unchanged)
    Frame rotated(double theta) const
    {
        const gp_Dir uRot{ gp_Vec{this->u} * std::cos(theta) + gp_Vec{this->v} * std::sin(theta) };
        const gp_Dir vRot{ gp_Vec{this->w} ^ gp_Vec{uRot} };
        return { uRot, vRot, this->w };
    }

    // Builds the local->WCS transformation: maps the local axes (DX,DY,DZ) onto the OCS axes
    gp_Trsf makeOcsToWcsTrsf() const
    {
        gp_Trsf trsf;
        trsf.SetDisplacement(
            gp_Ax3(gp::Origin(), gp::DZ(), gp::DX()), // local axes
            gp_Ax3(gp::Origin(), this->w, this->u)    // target OCS axes
        );
        return trsf;
    }

    // Builds the World Coordinate System frame from an OCS extrusion direction, following
    // AutoCAD's "Arbitrary Axis Algorithm"
    // Reference: https://help.autodesk.com/view/ACD/2024/ENU/?guid=GUID-4622B5B0-3D7B-4C7C-A1FD-A50461A1F46E
    // (or DXF Reference, "OCS to WCS Transformations")
    static Frame makeOcs(const gp_Dir& w)
    {
        constexpr double threshold = 1. / 64.;

        // Select the reference world axis depending on Az.x and Az.y
        const bool useWorldY = std::abs(w.X()) < threshold && std::abs(w.Y()) < threshold;
        const gp_Dir worldRef = useWorldY ? gp::DY() : gp::DZ();

        // Ax = worldRef × Az
        const gp_Vec axVec = gp_Vec(worldRef) ^ gp_Vec(w);

        // worldRef and Az are never colinear here: when useWorldY is true, |Az.x| and |Az.y|
        // are both < 1/64 so Az can't be parallel to DY; otherwise Az can't be parallel to DZ
        // (that would require |Az.x|,|Az.y| < 1/64, which is excluded by the branch condition)
        const gp_Dir u{axVec};

        // Ay = Az × Ax
        const gp_Dir v{gp_Vec(w) ^ gp_Vec(u)};

        return {u, v, w};
    }

    static const Frame& local()
    {
        static const Frame frame{gp::DX(), gp::DY(), gp::DZ()};
        return frame;
    }
};

struct Placement {
    gp_Ax2 ax2;
    Frame frame;

    static Placement makeFromOcs(
            const DxfCoords& centerOcs,
            const DxfCoords& extrusionDir,
            const gp_Pnt& originWcs = gp::Origin()
        )
    {
        Placement pl;
        pl.frame = Frame::makeOcs(toOccDir(extrusionDir));
        // Mapping OCS -> WCS
        const gp_Pnt cw = originWcs.Translated(
              gp_Vec{pl.frame.u} * centerOcs.x
            + gp_Vec{pl.frame.v} * centerOcs.y
            + gp_Vec{pl.frame.w} * centerOcs.z
        );
        // Z=w, X=u → deterministic orientation
        pl.ax2 = gp_Ax2{toCorrectedPnt(cw), pl.frame.w, pl.frame.u};
        return pl;
    }
};

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

// Is `scale` a unit-like triple{±1, ±1, ±1}?
bool isUnitScale(const DxfScale& scale)
{
    return    MathUtils::fuzzyEqual(std::abs(scale.x), 1.)
           && MathUtils::fuzzyEqual(std::abs(scale.y), 1.)
           && MathUtils::fuzzyEqual(std::abs(scale.z), 1.)
        ;
}

// Mirror/scale transform working purely in LOCAL coordinates
gp_Trsf unitScaleTrsf(const gp_Pnt& pnt, const Frame& frame, const DxfScale& scale)
{
    gp_Trsf trsf;
    assert(isUnitScale(scale));

    const double sx = scale.x;
    const double sy = scale.y;
    const double sz = scale.z;
    const int negativeCount = (sx < 0. ? 1 : 0) + (sy < 0. ? 1 : 0) + (sz < 0. ? 1 : 0);
    if (negativeCount == 0) {
    }
    else if (negativeCount == 1) {
        // Mirror normal to the corresponding axis
        const gp_Dir n = (sx < 0.) ? frame.u : ((sy < 0.) ? frame.v : frame.w);
        trsf.SetMirror(gp_Ax2(pnt, n));   // plane passing through `pnt` which normal `n`
    }
    else if (negativeCount == 2) {
        // Rotate by 180° around axis corresponding to positive sign
        const gp_Dir axis = (sx > 0.) ? frame.u : ((sy > 0.) ? frame.v : frame.w);
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
    return toCorrectedPnt(gp::Origin().Translated(
        gp_Vec{frame.u}*pnt.x + gp_Vec{frame.v}*pnt.y + gp_Vec{frame.w}*pnt.z
    ));
}

// OCS vector -> WCS(with ⟨u,v,w⟩ frame)
gp_Vec ocsVecToWcs(const DxfCoords& vec, const Frame& frame)
{
    return gp_Vec{frame.u}*vec.x + gp_Vec{frame.v}*vec.y + gp_Vec{frame.w}*vec.z;
}

std::string getEntityName(const Dxf_EntityVariant& entityVar)
{
    using namespace std::string_literals;
    return std::visit(Cpp::Overloaded{
        [](std::monostate) { return std::string{}; },
        [](ConstRefWrap<Dxf_3DFACE>) { return "3DFACE"s; },
        [](ConstRefWrap<Dxf_ARC>) { return "ARC"s; },
        [](ConstRefWrap<Dxf_CIRCLE>) { return "CIRCLE"s; },
        [](ConstRefWrap<Dxf_ELLIPSE>) { return "ELLIPSE"s; },
        [](ConstRefWrap<Dxf_INSERT> obj) { return "INSERT_" + std::string{obj.get().blockName}; },
        [](ConstRefWrap<Dxf_LINE>) { return "LINE"s; },
        [](ConstRefWrap<Dxf_LWPOLYLINE>) { return "LWPOLYLINE"s; },
        [](ConstRefWrap<Dxf_MTEXT>) { return "MTEXT"s; },
        [](ConstRefWrap<Dxf_POINT>) { return "POINT"s; },
        [](ConstRefWrap<Dxf_POLYLINE>) { return "POLYLINE"s; },
        [](ConstRefWrap<Dxf_SOLID>) { return "SOLID"s; },
        [](ConstRefWrap<Dxf_SPLINE>) { return "SPLINE"s; },
        [](ConstRefWrap<Dxf_TEXT>) { return "TEXT"s; },
        [](ConstRefWrap<Dxf_ATTRIB>) { return "ATTRIB"s; }
        }, entityVar
    );
}

DxfStringRef getEntityLayerName(const Dxf_EntityVariant& entityVar)
{
    DxfStringRef layerName = std::visit(Cpp::Overloaded{
        [](std::monostate) { return DxfStringRef{}; },
        [](auto cref) { return cref.get().layerName; }
        }, entityVar
    );
    return !layerName.empty() ? layerName : "0";
}

const Dxf_BaseEntity* getBaseEntityPtr(const Dxf_EntityVariant& entityVar)
{
    return std::visit(Cpp::Overloaded{
        [](std::monostate) { return static_cast<const Dxf_BaseEntity*>(nullptr); },
        [](auto cref) { return static_cast<const Dxf_BaseEntity*>(std::addressof(cref.get())); }
        }, entityVar
    );
}

DxfColorIndex findColorIndex(
        const Dxf_EntityVariant& entityVar, const Dxf_LAYER* dxfLayer, const Dxf_INSERT* dxfInsert
    )
{
    auto baseEntityPtr = getBaseEntityPtr(entityVar);
    if (baseEntityPtr) {
        if (baseEntityPtr->colorId == dxfColorByLayer && dxfLayer) {
            return dxfLayer->colorId;
        }
        else if (baseEntityPtr->colorId == dxfColorByBlock && dxfInsert) {
            return dxfInsert->colorId;
        }

        return baseEntityPtr->colorId;
    }

    return -1;
}

class TransferHelper {
public:
    explicit TransferHelper(DocumentPtr targetDoc)
        : m_doc(targetDoc)
    {}

    const TDF_LabelSequence& labelShapes() const
    {
        return m_shapeLabels;
    }

    TDF_Label addRootShape(
            const TopoDS_Shape& shape, std::string_view shapeName, const TDF_Label& layer
        )
    {
        const TDF_Label labelShape = m_doc->xcaf().shapeTool()->NewShape();
        m_doc->xcaf().shapeTool()->SetShape(labelShape, shape);
        TDataStd_Name::Set(labelShape, to_OccExtString(shapeName));
        m_shapeLabels.Append(labelShape);
        if (!layer.IsNull())
            m_doc->xcaf().layerTool()->SetLayer(labelShape, layer, true/*onlyInOneLayer*/);

        return labelShape;
    }

    TDF_Label getOrAddLayerLabel(DxfStringRef dxfLayerName)
    {
        TDF_Label layerLabel;
        auto it = m_mapLayerNameLabel.find(dxfLayerName);
        if (it == m_mapLayerNameLabel.cend()) {
            layerLabel = m_doc->xcaf().layerTool()->AddLayer(to_OccExtString(dxfLayerName));
            m_mapLayerNameLabel.try_emplace(dxfLayerName, layerLabel);
        }
        else {
            layerLabel = it->second;
        }

        return layerLabel;
    }

    TDF_Label getOrAddColorLabel(DxfColorIndex aci)
    {
        auto it = m_mapColorIndexLabel.find(aci);
        if (it != m_mapColorIndexLabel.cend())
            return it->second;

        if (0 <= aci && Cpp::cmpLess(aci, std::size(aciTable))) {
            const RGB_Color& c = aciTable[aci].second;
            const TDF_Label colorLabel = m_doc->xcaf().colorTool()->AddColor(
                Quantity_Color(c.r / 255., c.g / 255., c.b / 255., Quantity_TOC_RGB)
            );
            m_mapColorIndexLabel.insert({ aci, colorLabel });
            return colorLabel;
        }

        return TDF_Label{};
    }

    void setShapeColor(const TDF_Label& labelShape, DxfColorIndex aci)
    {
        const TDF_Label labelColor = getOrAddColorLabel(aci);
        if (!labelColor.IsNull())
            m_doc->xcaf().colorTool()->SetColor(labelShape, labelColor, XCAFDoc_ColorGen);
    }

private:
    std::unordered_map<DxfStringRef, TDF_Label> m_mapLayerNameLabel;
    std::unordered_map<DxfColorIndex, TDF_Label> m_mapColorIndexLabel;
    DocumentPtr m_doc;
    TDF_LabelSequence m_shapeLabels;
};

} // namespace

class DxfReader::ReaderImpl : public DxfParser {
public:
    ReaderImpl();

    bool read(const FilePath& filepath, TaskProgress* progress = nullptr);
    void setMessenger(Messenger* messenger) { m_messenger = messenger; }
    void setParameters(const DxfReader::Parameters& params) { m_params = params; }

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
    TopoDS_Shape createShape(const Dxf_ATTRIB& attrib);

    std::string toUtf8(const std::string& strSource) const;

    static TopoDS_Shape makeFace(
        const Dxf_QuadBase& quad,
        const std::function<bool(int)>& isEdgeVisible = nullptr
    );

private:
    bool setSourceEncoding(std::string_view codepage);

    static TopoDS_Shape createSplineFromPolesAndKnots(const Dxf_SPLINE& spline);
    static TopoDS_Shape createInterpolationSpline(const Dxf_SPLINE& spline);

    TopoDS_Shape createShapePolygonMesh3d(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapePolyfaceMesh(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapePolyline3d(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapePolyline2d(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapeCurveFit(const Dxf_POLYLINE& polyline);
    TopoDS_Shape createShapeSplineFit(const Dxf_POLYLINE& polyline);

    // Create OCC curve corresponding to arc via bulge
    static OccHandle<Geom_TrimmedCurve> makeArcFromBulge(
        const gp_Pnt& p0, const gp_Pnt& p1, double bulge, const gp_Dir& planeNormal
    );

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
    explicit Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->importAnnotations.setDescription(
            textIdTr("Import text/dimension objects"));
        this->groupLayers.setDescription(
            textIdTr("Group all objects within a layer into a single compound shape"));
        this->fontNameForTextObjects.setDescription(
            textIdTr("Name of the font to be used when creating shape for text objects"));
    }

    void restoreDefaults() override {
        const DxfReader::Parameters params;
        this->importAnnotations.setValue(params.importAnnotations);
        this->groupLayers.setValue(params.groupLayers);
        this->fontNameForTextObjects.setValue(0);
    }

    PropertyBool importAnnotations{ this, textId("importAnnotations") };
    PropertyBool groupLayers{ this, textId("groupLayers") };
    PropertyEnumeration fontNameForTextObjects{ this, textId("fontNameForTextObjects"), &systemFontNames() };
};

DxfReader::DxfReader() = default;

DxfReader::~DxfReader() = default;

bool DxfReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    m_impl = std::make_unique<DxfReader::ReaderImpl>();
    m_impl->setParameters(m_params);
    m_impl->setMessenger(this->messenger() ? this->messenger() : &Messenger::null());
    return m_impl->read(filepath, progress);
}

TDF_LabelSequence DxfReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (!m_impl)
        return {};

    if (m_params.groupLayers)
        return this->transferByGroupLayers(doc, progress);
    else
        return this->transferBySingleEntities(doc, progress);
}

std::unique_ptr<PropertyGroup> DxfReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void DxfReader::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.importAnnotations = ptr->importAnnotations;
        m_params.groupLayers = ptr->groupLayers;
        m_params.fontNameForTextObjects = ptr->fontNameForTextObjects.valueName();
    }
}

DxfReader::ReaderImpl::ReaderImpl()
{
    this->setGetLinePostCallback([=](size_t getLineSize) {
        ++m_lineCounter;
        m_fileReadSize += getLineSize;
        if (m_progress) {
            m_progress->setValue(MathUtils::toPercent(m_fileReadSize, 0, m_fileSize));
        }
    });
    this->setReportErrorCallback([=](std::string_view msg) {
        m_messenger->emitError(msg);
    });
}

bool DxfReader::ReaderImpl::read(const FilePath& filepath, TaskProgress* progress)
{
    std::ifstream fstr(filepath);
    if (!fstr.is_open())
        return false;

    m_fileSize = filepathFileSize(filepath);
    m_progress = progress;
    this->parse(fstr);
    this->setSourceEncoding(this->codePage());
    return !this->failed();
}

bool DxfReader::ReaderImpl::setSourceEncoding(std::string_view codepage)
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
        m_messenger->emitWarning("Codepage " + std::string{codepage} + " not supported");
    }

    return true;
}

std::string DxfReader::ReaderImpl::toUtf8(const std::string& strSource) const
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

TopoDS_Shape DxfReader::ReaderImpl::createBlockShape(const Dxf_BLOCK& block)
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

TopoDS_Shape DxfReader::ReaderImpl::createEntityShape(const Dxf_EntityVariant& entityVar)
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
        [=](ConstRefWrap<Dxf_TEXT> obj) { return createShape(obj); },
        [=](ConstRefWrap<Dxf_ATTRIB> obj) { return createShape(obj); },
    }, entityVar);
    return entityShape;
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_3DFACE& face)
{
    try {
        auto isEdgeVisible = [&](int index) {
            switch (index) {
            case 1: return (face.flags & Dxf_3DFACE::InvisibleEdge1) == 0;
            case 2: return (face.flags & Dxf_3DFACE::InvisibleEdge2) == 0;
            case 3: return (face.flags & Dxf_3DFACE::InvisibleEdge3) == 0;
            case 4: return (face.flags & Dxf_3DFACE::InvisibleEdge4) == 0;
            default: return true;
            }
        };
        return makeFace(face, isEdgeVisible);
    }
    catch (...) {
        m_messenger->emitError("createShape(3DFACE) failed");
        return {};
    }
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_ARC& arc)
{
    if (arc.radius <= Precision::Confusion()) {
        m_messenger->emitError("ARC radius is null or degenerated");
        return {};
    }

    // Point on circle from OCS angle(radians)
    // P(θ) = C + R*(cosθ * u + sinθ * v)
    auto pointOnCircle = [](const gp_Pnt& C, double R, double theta, const gp_Dir& u, const gp_Dir& v) {
        return C.Translated(gp_Vec{u} * (R * std::cos(theta)) + gp_Vec{v} * (R * std::sin(theta)));
    };

    const auto pl = Placement::makeFromOcs(arc.centerPoint, arc.extrusionDirection);

    const gp_Circ circ{pl.ax2, arc.radius};

    // Normalize angle `a` in degrees within [0,360)
    auto normalizeAngleDeg = [](double a) -> double {
        double r = std::fmod(a, 360.);
        if (r < 0.0)
            r += 360.0;
        return r;
    };

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

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_CIRCLE& circle)
{
    if (circle.radius <= Precision::Confusion()) {
        m_messenger->emitError("CIRCLE radius is null or quasi null");
        return {};
    }

    const auto pl = Placement::makeFromOcs(circle.centerPoint, circle.extrusionDirection);
    const gp_Circ circ{pl.ax2, circle.radius};
    return makeExtrusionShape(BRepBuilderAPI_MakeEdge(circ), circle.thickness, pl.frame.w);
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_ELLIPSE& ellipse)
{
    if (ellipse.ratioMinorMajorAxis <= Precision::Confusion()) {
        m_messenger->emitError("ELLIPSE ratio is null or quasi null");
        return {};
    }

    auto pl = Placement::makeFromOcs(ellipse.centerPoint, ellipse.extrusionDirection);

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

    constexpr double pi2 = 2 * 3.14159265358979323846;
    auto normalizeAngleRad = [=](double a) {
        double r = std::fmod(a, pi2); // Force in  [0, 2π)
        if (r < 0.)
            r += pi2;
        return r;
    };

    // Build edge, complete or partial
    TopoDS_Edge edge;
    const double u1 = normalizeAngleRad(ellipse.startParam);
    const double u2 = normalizeAngleRad(ellipse.endParam);
    const double uDiff = u2 - u1;
    const double uDiffMod = uDiff > 0. ? uDiff : (uDiff + pi2);
    // Full ellipse if uDiffMod~0 or uDiffMod~2π
    if (MathUtils::fuzzyIsNull(uDiffMod) || MathUtils::fuzzyIsNull(uDiffMod - pi2))
        edge = BRepBuilderAPI_MakeEdge(elips);
    else
        edge = BRepBuilderAPI_MakeEdge(elips, u1, u1 + uDiffMod);

    return makeExtrusionShape(edge, ellipse.thickness, pl.frame.w);
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_INSERT& insert)
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
    const Frame frame = Frame::makeOcs(toOccDir(insert.extrusionDirection));

    // Parameters for grid-like insertion
    const int rows = std::max(1, insert.rowCount);
    const int cols = std::max(1, insert.columnCount);

    // Check if uniform scale
    const bool unitScale = isUnitScale(insert.scaleFactor);

    // Create shape for the result
    TopoDS_Compound result = BRepUtils::makeEmptyCompound();

    // Common pre-computations
    const gp_Pnt insertPnt_wcs = ocsPointToWcs(insert.insertPoint, frame);
    const double theta = MathUtils::degreeToRadian(insert.rotationAngle);
    const Frame rotFrame = frame.rotated(theta);

    auto gridCellPoint = [&](int row, int col) -> gp_Pnt {
        const gp_Vec offset =
              gp_Vec{rotFrame.u} * (insert.rowSpacing * row)
            + gp_Vec{rotFrame.v} * (insert.columnSpacing * col)
        ;
        return insertPnt_wcs.Translated(offset);
    };

    if (unitScale) {
        const gp_Trsf scaleTrsf = unitScaleTrsf(gp::Origin(), Frame::local(), insert.scaleFactor);
        const gp_Trsf frameTrsf = rotFrame.makeOcsToWcsTrsf();
        const gp_Trsf t1Trsf = GeomUtils::makeTranslation(-toOccVec(block.basePoint));
        for (int j = 0; j < rows; ++j) {
            for (int i = 0; i < cols; ++i) {
                const gp_Trsf t2Trsf = GeomUtils::makeTranslation(gridCellPoint(j, i).XYZ());
                const gp_Trsf trsf = t2Trsf * frameTrsf * scaleTrsf * t1Trsf;
                BRepUtils::addShape(&result, content.Moved(trsf));
            }
        }
    }
    else {
        // Non-unit scale -> GTransform
        // L = R(theta) * matFrame * matScale  (scale in LOCAL axes, then OCS basis change, then rotation

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
        const gp_Trsf R = GeomUtils::makeRotation(gp_Ax1{gp::Origin(), frame.w}, theta);
        const gp_Mat L = R.VectorialPart() * matFrame * matScale;
        const gp_XYZ LB = L * toOccVec(block.basePoint).XYZ();
        for (int j = 0; j < rows; ++j) {
            for (int i = 0; i < cols; ++i) {
                gp_GTrsf G;
                G.SetVectorialPart(L);
                G.SetTranslationPart(gridCellPoint(j, i).XYZ() - LB);
                BRepUtils::addShape(&result, BRepBuilderAPI_GTransform(content, G, true/*copy*/).Shape());
            }
        }
    }

    return result;
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_LINE& line)
{
    const Frame frame = Frame::makeOcs(toOccDir(line.extrusionDirection));
    const gp_Pnt p1 = ocsPointToWcs(line.startPoint, frame);
    const gp_Pnt p2 = ocsPointToWcs(line.endPoint, frame);
    if (GeomUtils::equal(p1, p2))
        return {};

    return makeExtrusionShape(BRepBuilderAPI_MakeEdge(p1, p2), line.thickness, frame.w);
}

OccHandle<Geom_TrimmedCurve> DxfReader::ReaderImpl::makeArcFromBulge(
        const gp_Pnt& p0, const gp_Pnt& p1, double bulge, const gp_Dir& planeNormal
    )
{
    // Chord
    const gp_Vec chord(p0, p1);
    const double c = chord.Magnitude();
    if (c <= gp::Resolution() || MathUtils::fuzzyIsNull(bulge))
        return {};

    // Chord direction and perpendicular vec in the plane
    const gp_Vec d = chord / c;
    gp_Vec perp = gp_Vec{planeNormal} ^ d; // n × chordDir
    if (GeomUtils::isNull(perp))
        return {};

    perp.Normalize();

    // Signed angle and geom parameters
    const double theta = 4.0 * std::atan(bulge); // CCW>0, CW<0

    // Radius and offset center from middle of the chord
    const double sinHalf = std::sin(0.5 * theta);
    const double tanHalf = std::tan(0.5 * theta);
    if (MathUtils::fuzzyIsNull(sinHalf) || MathUtils::fuzzyIsNull(tanHalf))
        return {};

    const double R = std::abs((0.5 * c) / sinHalf);
    const double h = (0.5 * c) / tanHalf; // Signed(same as theta)

    // Middle and center
    const gp_Pnt mid = p0.Translated(d * (0.5 * c));
    const gp_Pnt center = mid.Translated(perp * h);

    // Circle in the plane, positive radius
    const gp_Circ circ(gp_Ax2{center, planeNormal}, R);

    // p0 and p1 parameters on the circle
    const double a0 = ElCLib::Parameter(circ, p0);
    const double a1 = a0 + theta;

    // Trimmed arc
    auto geom = makeOccHandle<Geom_Circle>(circ);
    auto curve = makeOccHandle<Geom_TrimmedCurve>(geom, std::min(a0, a1), std::max(a0, a1));
    if (a1 < a0)
        curve->Reverse();

    return curve;
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_LWPOLYLINE& polyline)
{
    const size_t n = polyline.vertices.size();
    if (n < 2)
        return {};

    const bool isClosed = (polyline.flag & 1u) != 0u;
    const auto pl = Placement::makeFromOcs({0., 0., polyline.elevation}, polyline.extrusionDirection);
    const gp_Dir& normal = pl.ax2.Direction();

    auto makePolar = [&](const Dxf_LWPOLYLINE::Vertex& vertex) -> gp_Pnt {
        const gp_Vec vec = gp_Vec{pl.frame.u} * vertex.x + gp_Vec{pl.frame.v} * vertex.y;
        return pl.ax2.Location().Translated(vec);
    };

    BRepBuilderAPI_MakeWire makeWire;
    auto addEdge = [&](const gp_Pnt& p0, const gp_Pnt& p1, double bulge) {
        if (GeomUtils::equal(p0, p1))
            return;

        if (MathUtils::fuzzyIsNull(bulge)) {
            makeWire.Add(BRepBuilderAPI_MakeEdge(p0, p1));
        } else {
            OccHandle<Geom_TrimmedCurve> arc = makeArcFromBulge(p0, p1, bulge, normal);
            if (!arc.IsNull())
                makeWire.Add(BRepBuilderAPI_MakeEdge(arc));
            else
                makeWire.Add(BRepBuilderAPI_MakeEdge(p0, p1));
        }
    };

    for (size_t i = 0; i + 1 < n; ++i) {
        const auto& v0 = polyline.vertices.at(i);
        const auto& v1 = polyline.vertices.at(i+1);
        addEdge(makePolar(v0), makePolar(v1), v0.bulge);
    }

    if (isClosed) {
        const auto& vLast = polyline.vertices.at(n - 1);
        const auto& vFirst = polyline.vertices.at(0);
        addEdge(makePolar(vLast), makePolar(vFirst), vLast.bulge);
    }

    if (!makeWire.IsDone())
        return {}; // TODO Emit error message?

    return makeExtrusionShape(makeWire.Wire(), polyline.thickness, pl.frame.w);
}

std::string DxfReader::getPlainMText(std::string_view strMText)
{
    auto strReplaceAll = [](std::string* str, std::string_view strBefore, std::string_view strAfter) {
        size_t pos = str->find(strBefore, 0);
        while (pos != std::string::npos) {
            str->replace(pos, strBefore.size(), strAfter);
            pos = str->find(strBefore, pos + strAfter.size());
        }
    };

    std::string out{strMText};

    static const std::regex openCloseBraceRx(R"((\{\s*)|(\s*\}))");
    out = std::regex_replace(out, openCloseBraceRx, "");

    // Commands
    static const std::regex cmdRx(R"(\\[HWSACcfp][\^,/0-9a-zA-Z\\. \\|_]*;)");
    out = std::regex_replace(out, cmdRx, "");
    static const std::regex cmdSimpleRx(R"(\\[LlOo];?)");
    out = std::regex_replace(out, cmdSimpleRx, "");
    strReplaceAll(&out, "\\P", "\n");

    // Caret-codes
    strReplaceAll(&out, "^I", "\t");
    strReplaceAll(&out, "^J", "\n");
    strReplaceAll(&out, "^M", "");

    // Unicode sequence \U+nnnn
    static const std::regex useqRx(R"(\\U\+([0-9A-Fa-f]{4}))");
    {
        std::smatch match;
        std::smatch::difference_type pos = 0;
        while (std::regex_search(out.cbegin() + pos, out.cend(), match, useqRx)) {
            pos += match.position(0);
            const std::string strCode = match.str(1);
            // Parse 4 hexadecimal digits(eg 00B0) into an unsigned integer
            // This matches the MTEXT escape form \U+nnnn, which is always 4 hex digits(BMP)
            // NOTE fromChars() wants an integer type(refuses char16_t)
            uint16_t code = 0;
            Mayo::fromChars(strCode, code, 16);
            out.replace(pos, match.length(0), toUtf8(code).data());
        }
    }

    DxfReader::replaceTextControlCodes(&out);
    return out;
}

TDF_LabelSequence DxfReader::transferByGroupLayers(DocumentPtr doc, TaskProgress* progress)
{
    struct TransferObject {
        const Dxf_EntityVariant* entityVariantPtr{nullptr};
        TopoDS_Shape shape;
        DxfColorIndex explicitColorId{-1};
    };
    using ArrayOfTransferObjects = std::vector<TransferObject>;

    TransferHelper transferHelper(doc);
    std::unordered_map<const Dxf_LAYER*, ArrayOfTransferObjects> mapObjectsByLayer;

    for (const Dxf_EntityVariant& entityVar : m_impl->allEntities()) {
        const TopoDS_Shape entityShape = m_impl->createEntityShape(entityVar);
        if (entityShape.IsNull())
            continue; // Skip

        const Dxf_LAYER* dxfLayer = m_impl->findLayer(getEntityLayerName(entityVar));
        dxfLayer = !dxfLayer ? m_impl->findLayer("0") : dxfLayer;
        assert(dxfLayer != nullptr);

        TransferObject object;
        object.entityVariantPtr = &entityVar;
        object.shape = entityShape;
        object.explicitColorId = findColorIndex(entityVar, dxfLayer, nullptr);

        auto& arrayOfTransferObjects = mapObjectsByLayer[dxfLayer];
        arrayOfTransferObjects.push_back(std::move(object));
        progress->setValue(MathUtils::toPercent(
            Cpp::indexInSpan(m_impl->allEntities(), entityVar), 0, m_impl->allEntities().size()
        ));
    }

    for (const auto& [layer, objects] : mapObjectsByLayer) {
        TopoDS_Shape layerShape;
        for (const TransferObject& obj : objects)
            BRepUtils::addShape(&layerShape, obj.shape);

        assert(!layerShape.IsNull());
        assert(!objects.empty());

        const TDF_Label layerLabel = transferHelper.getOrAddLayerLabel(layer->name);
        const TDF_Label shapeLabel = transferHelper.addRootShape(layerShape, layer->name, layerLabel);
        // Check if all entities have the same color
        const DxfColorIndex aci = objects.front().explicitColorId;
        const bool uniqueColor = std::all_of(objects.cbegin(), objects.cend(), [=](const TransferObject& obj) {
            return obj.explicitColorId == aci;
        });

        if (uniqueColor) {
            transferHelper.setShapeColor(shapeLabel, aci);
        }
        else {
            for (const TransferObject& obj : objects) {
                assert(!obj.shape.IsNull());
                const TDF_Label objShapeLabel = doc->xcaf().shapeTool()->AddSubShape(shapeLabel, obj.shape);
                transferHelper.setShapeColor(objShapeLabel, obj.explicitColorId);
            }
        }
    }

    return transferHelper.labelShapes();
}

TDF_LabelSequence DxfReader::transferBySingleEntities(DocumentPtr doc, TaskProgress* progress)
{
    TransferHelper transferHelper(doc);

    std::unordered_map<size_t, unsigned> mapCountByEntityType;

    for (const Dxf_EntityVariant& entityVar : m_impl->allEntities()) {
        const TopoDS_Shape entityShape = m_impl->createEntityShape(entityVar);
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

            strEntityName += fmt::format("_{}", iShape);
        }

        // Move entity in layer
        DxfStringRef dxfLayerName = getEntityLayerName(entityVar);
        const TDF_Label layerLabel = transferHelper.getOrAddLayerLabel(dxfLayerName);
        const TDF_Label shapeLabel = transferHelper.addRootShape(entityShape, strEntityName, layerLabel);
        // Assign color
        const Dxf_LAYER* dxfLayer = m_impl->findLayer(dxfLayerName);
        transferHelper.setShapeColor(shapeLabel, findColorIndex(entityVar, dxfLayer, nullptr));

        progress->setValue(MathUtils::toPercent(
            Cpp::indexInSpan(m_impl->allEntities(), entityVar), 0, m_impl->allEntities().size()
        ));
    }

    return transferHelper.labelShapes();
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_MTEXT& mtext)
{
    if (!m_params.importAnnotations)
        return {};

    const gp_Dir extrusionDir = toOccDir(mtext.extrusionDirection);
    const Frame frame = Frame::makeOcs(extrusionDir);
    const gp_Pnt pt = ocsPointToWcs(mtext.insertionPoint, frame);
    const std::string& fontName = m_params.fontNameForTextObjects;
    const double lineHeight = 1.4 * mtext.height;
    Font_BRepFont brepFont;
    if (!brepFont.Init(fontName.c_str(), Font_FA_Regular, lineHeight)) {
        m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        return {};
    }

    const auto ap = mtext.attachmentPoint;
    using AttachPnt = Dxf_MTEXT::AttachmentPoint;
    Graphic3d_HorizontalTextAlignment hAlign = Graphic3d_HTA_LEFT;
    if (ap == AttachPnt::TopCenter || ap == AttachPnt::MiddleCenter || ap == AttachPnt::BottomCenter)
        hAlign = Graphic3d_HTA_CENTER;
    else if (ap == AttachPnt::TopRight || ap == AttachPnt::MiddleRight || ap == AttachPnt::BottomRight)
        hAlign = Graphic3d_HTA_RIGHT;

    Graphic3d_VerticalTextAlignment vAlign = Graphic3d_VTA_TOP;
    if (ap == AttachPnt::MiddleLeft || ap == AttachPnt::MiddleCenter || ap == AttachPnt::MiddleRight)
        vAlign = Graphic3d_VTA_CENTER;
    else if (ap == AttachPnt::BottomLeft || ap == AttachPnt::BottomCenter || ap == AttachPnt::BottomRight)
        vAlign = Graphic3d_VTA_BOTTOM;

    // Ensure non-null x-axis direction
    gp_Vec xAxisDir = toOccVec(mtext.xAxisDirection);
    if (GeomUtils::isNull(xAxisDir))
        xAxisDir = gp::DX();

    // If rotation angle is non-null and x-axis direction defaults to standard Ox then set x-axis
    // so it matches rotation angle
    xAxisDir.Normalize();
    if (!MathUtils::fuzzyIsNull(mtext.rotationAngle) && GeomUtils::equal(xAxisDir, gp::DX())) {
        const double angle = MathUtils::degreeToRadian(mtext.rotationAngle);
        const gp_Trsf trsf = GeomUtils::makeRotation(gp_Ax1(pt, gp::DZ()), angle);
        xAxisDir = gp::DX().Transformed(trsf);
    }

    const std::string mTextStr_u8 = this->toUtf8(DxfReader::getPlainMText(mtext.str));
    const auto occTextStr = string_conv<NCollection_String>(mTextStr_u8);
    const gp_Dir extDir = toOccDir(mtext.extrusionDirection);
    const gp_Ax3 locText(pt, extDir, xAxisDir);
    Font_BRepTextBuilder brepTextBuilder;
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)    
    auto textFormat = makeOccHandle<Font_TextFormatter>();

    // Enable word wrapping only if text contains spaces or tabs
    bool strHasWordSeparators = false;
    for (int i = 0; i < occTextStr.Length(); ++i) {
        if (occTextStr.GetChar(i) == ' ' || occTextStr.GetChar(i) == '\x09'/*tab*/) {
            strHasWordSeparators = true;
            break;
        }
    }

    if (strHasWordSeparators) {
        brepFont.FTFont()->RenderGlyph(U'M');
        Font_Rect fontRect;
        brepFont.FTFont()->GlyphRect(fontRect);
        const double factor = fontRect.Height() / mtext.height;
        textFormat->SetWrapping(float(mtext.referenceRectangleWidth * factor));
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 7, 0)
        textFormat->SetWordWrapping(true);
#endif
    }

    textFormat->SetupAlignment(hAlign, vAlign);
    textFormat->Append(occTextStr, *brepFont.FTFont());
    textFormat->Format();
    TopoDS_Shape shape = brepTextBuilder.Perform(brepFont, textFormat, locText);
#else
    TopoDS_Shape shape = brepTextBuilder.Perform(brepFont, occTextStr, locText, hAlign, vAlign);
#endif

    // Empirical correction, this makes the shape closer to what's expected(compared to ezdxf)
    if (vAlign == Graphic3d_VTA_TOP)
        shape.Move(GeomUtils::makeTranslation(gp_Vec{locText.YDirection()} * lineHeight * 0.18));

    return shape;
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_POINT& point)
{
    TopoDS_Shape shape = BRepUtils::makeEmptyCompound();

    const gp_Pnt pnt = toOccPnt(point.location);
    const gp_Dir n = toOccDir(point.extrusionDirection);
    // If thickness != 0 → extruded segment
    if (!MathUtils::fuzzyIsNull(point.thickness)) {
        const gp_Vec step(n.XYZ() * point.thickness);
        const gp_Pnt dst(pnt.X() + step.X(), pnt.Y() + step.Y(), pnt.Z() + step.Z());
        if (!GeomUtils::equal(pnt, dst))
            BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(pnt, dst));
    }

    const int pdmode = dxfGetInt(this->headerVariableValue("PDMODE"), 0);
    double pdsize = dxfGetDouble(this->headerVariableValue("PDSIZE"), 0.);
    if (pdsize > 0.)
        pdsize = this->mm(pdsize);

    // Interpret PDMODE/PDSIZE variables
    const bool drawCircle = (pdmode & 32) != 0;
    const bool drawSquare = (pdmode & 64) != 0;
    const int base = pdmode & 0x1F; // 0..31; base values 0,1,2,3,4

    gp_Ax2 ax2 = Placement::makeFromOcs(point.location, point.extrusionDirection).ax2;
    const double angleXAxis = pdmode != 0 ? point.angleXAxis : 0.;
    if (!MathUtils::fuzzyIsNull(point.angleXAxis)) {
        const double angleXAxis_rad = MathUtils::degreeToRadian(angleXAxis);
        ax2.Transform(GeomUtils::makeRotation(gp_Ax1(pnt, ax2.Direction()), angleXAxis_rad));
    }

    // Symbol size according PDSIZE

    // Resolve symbol size accoding PDSIZE and an optional viewport height
    // - pdsize > 0 : absolute size -> returned as is
    // - pdsize == 0 : 5% of viewportHeight if provided, fallback(1.) otherwise
    // - pdsize < 0 : (-pdsize)% of viewportHeight if provided, fallback(|pdsize|) otherwise
    auto resolvePointSizeInModelUnits = [](
            double pdsize, std::optional<double> viewportHeight, double fallbackAbsSize = 1.
        )
    {
        if (pdsize > 0.)
            return pdsize;

        const double vpHeight = viewportHeight.value_or(-1.);
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
        // 2, 3, 4 : "linear" shapes
        switch (base) {
        case 2: // cross '+'
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), ax2.XDirection(), L));
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), ax2.YDirection(), L));
            break;
        case 3: { // cross 'X'
            const gp_Vec u{ax2.XDirection()};
            const gp_Vec v{ax2.YDirection()};
            const gp_Vec diagPlus = u.Added(v).Normalized();
            const gp_Vec diagMinus = u.Subtracted(v).Normalized();
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), diagPlus, L));
            BRepUtils::addShape(&shape, makeCenteredSegment(ax2.Location(), diagMinus, L));
        }
            break;
        case 4: // tick '✓'
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
        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(gp_Circ{ax2, 0.5 * L});
        BRepUtils::addShape(&shape, BRepBuilderAPI_MakeWire(edge));
    }

    if (drawSquare) {
        const gp_Vec ux{ax2.XDirection()};
        const gp_Vec vy{ax2.YDirection()};
        const double h = 0.5 * L;

        const gp_Pnt p1 = ax2.Location().Translated(-h*ux - h*vy);
        const gp_Pnt p2 = ax2.Location().Translated( h*ux - h*vy);
        const gp_Pnt p3 = ax2.Location().Translated( h*ux + h*vy);
        const gp_Pnt p4 = ax2.Location().Translated(-h*ux + h*vy);

        BRepBuilderAPI_MakeWire makeWire;
        makeWire.Add(BRepBuilderAPI_MakeEdge(p1, p2));
        makeWire.Add(BRepBuilderAPI_MakeEdge(p2, p3));
        makeWire.Add(BRepBuilderAPI_MakeEdge(p3, p4));
        makeWire.Add(BRepBuilderAPI_MakeEdge(p4, p1));
        BRepUtils::addShape(&shape, makeWire.Wire());
    }

    return shape;
}

TopoDS_Shape DxfReader::ReaderImpl::createShapePolygonMesh3d(const Dxf_POLYLINE& polyline)
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

TopoDS_Shape DxfReader::ReaderImpl::createShapePolyfaceMesh(const Dxf_POLYLINE& polyline)
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

TopoDS_Shape DxfReader::ReaderImpl::createShapePolyline3d(const Dxf_POLYLINE& polyline)
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

TopoDS_Shape DxfReader::ReaderImpl::createShapePolyline2d(const Dxf_POLYLINE& polyline)
{
    // TODO Handle Dxf_POLYLINE::Vertex::bulge
    const auto& vertices = polyline.vertices;
    const bool isPolylineClosed = (polyline.flags & Dxf_POLYLINE::Flag::Closed) != 0;
    const int nodeCount = gsl::narrow<int>(vertices.size() + (isPolylineClosed ? 1 : 0));
    const gp_Dir extrusionDir = toOccDir(polyline.extrusionDirection);
    const Frame frame = Frame::makeOcs(extrusionDir);
    MeshUtils::Polygon3dBuilder polygonBuilder(nodeCount);
    for (unsigned i = 0; i < vertices.size(); ++i)
        polygonBuilder.setNode(i + 1, ocsPointToWcs(vertices.at(i).point, frame));

    if (isPolylineClosed)
        polygonBuilder.setNode(nodeCount, ocsPointToWcs(vertices.at(0).point, frame));

    polygonBuilder.finalize();
    const TopoDS_Shape shape = BRepUtils::makeEdge(polygonBuilder.get());
    return makeExtrusionShape(shape, polyline.thickness, extrusionDir);
}

TopoDS_Shape DxfReader::ReaderImpl::createShapeCurveFit(const Dxf_POLYLINE& polyline)
{
    const auto& vertices = polyline.vertices;
    const bool isPolylineClosed = polyline.flags & Dxf_POLYLINE::Flag::Closed;
    const int nodeCount = gsl::narrow<int>(vertices.size() + (isPolylineClosed ? 1 : 0));
    const gp_Dir extrusionDir = toOccDir(polyline.extrusionDirection);
    const Frame frame = Frame::makeOcs(extrusionDir);
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
            if (!GeomUtils::isNull(dir))
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

TopoDS_Shape DxfReader::ReaderImpl::createShapeSplineFit(const Dxf_POLYLINE& polyline)
{
    const bool isPolylineClosed = polyline.flags & Dxf_POLYLINE::Flag::Closed;
    const gp_Dir extrusionDir = toOccDir(polyline.extrusionDirection);
    const Frame frame = Frame::makeOcs(extrusionDir);

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

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_POLYLINE& polyline)
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

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_SOLID& solid)
{
    const Dxf_QuadBase* quadPtr = &solid;
    Dxf_QuadBase quadLocal; // Needed only if solid has a fourth corner

    if (solid.hasCorner4) {
        // See https://ezdxf.readthedocs.io/en/stable/dxfentities/solid.html
        quadLocal = static_cast<const Dxf_QuadBase&>(solid);
        std::swap(quadLocal.corner3, quadLocal.corner4);
        quadPtr = &quadLocal;
    }

    try {
        return makeFace(*quadPtr);
    }
    catch (...) {
        m_messenger->emitError("createShape(Dxf_SOLID) failed");
        return {};
    }
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_SPLINE& spline)
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

void DxfReader::replaceTextControlCodes(std::string* str)
{
    auto toStrControlCode = [](char ch) -> std::pair<const char*, unsigned> {
        switch (ch) {
        case 'd': return {"°", 2u}; // 2bytes
        case 'c': return {"Ø", 2u}; // 2bytes
        case 'p': return {"±", 2u}; // 2bytes
        case '%': return {"%", 1u}; // 1byte
        case 'o': return {"", 0u}; // Overscoring on/off
        case 'u': return {"", 0u}; // Underscoring on/off
        case 'k': return {"", 0u}; // Strikethrough
        default: return {nullptr, 0u};
        }
    };

    size_t pos = str->find("%%", 0);
    while (pos != std::string::npos) {
        size_t offset = 2;
        if (pos + 2 < str->length()) {
            const char ch = toLowerCase_C(str->at(pos + 2));
            const auto [r, rlen] = toStrControlCode(ch);
            if (r != nullptr) {
                str->replace(pos, 3, r);
                offset = rlen;
            }
            else if (isDigit_C(ch) && pos + 4 < str->length()) {
                const char ch1 = str->at(pos + 3);
                const char ch2 = str->at(pos + 4);
                if (isDigit_C(ch1) && isDigit_C(ch2)) {
                    str->replace(pos, 5, "?");
                    offset = 1;
                }
            }
        }

        pos = str->find("%%", pos + offset);
    }
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_TEXT& text)
{
    if (!m_params.importAnnotations)
        return {};

    const Dxf_STYLE* ptrStyle = this->findStyle(text.styleName);
    std::string fontName = ptrStyle ? std::string{ptrStyle->name} : m_params.fontNameForTextObjects;
    // "ARIAL_NARROW" -> "ARIAL NARROW"
    if (toLowerCase_C(fontName) == "arial_narrow")
        fontName.replace(5, 1, " ");

    const double fontHeight = 1.4 * text.height;
    Font_BRepFont brepFont;
    if (!brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight)) {
        m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        return {};
    }

    const bool xMirror = (text.generationFlags & 0x02) != 0;
    const bool yMirror = (text.generationFlags & 0x04) != 0;

    const gp_Dir extDir = toOccDir(text.extrusionDirection);
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
    if (vjust == DxfVJustification::Baseline || vjust == DxfVJustification::Bottom)
        vAlign = Graphic3d_VTA_BOTTOM;
    else if (vjust == DxfVJustification::Middle)
        vAlign = Graphic3d_VTA_CENTER;

    // Alignment point
    const gp_Pnt alignPnt1 = toOccPnt(text.firstAlignmentPoint);
    const gp_Pnt alignPnt2 = toOccPnt(text.secondAlignmentPoint);
    const bool useFirstAlignPnt =
        (hjust == DxfHJustification::Fit && !xMirror)
        || (hjust == DxfHJustification::Aligned && !xMirror)
        || (hjust == DxfHJustification::Left && vjust == DxfVJustification::Baseline)
    ;
    const gp_Pnt pnt = useFirstAlignPnt ? alignPnt1 : alignPnt2;

    // X axis(firstAlignmentPoint ? secondAlignmentPoint)
    gp_Vec xAxisDir = gp::DX();
    if (hjust == DxfHJustification::Aligned || hjust == DxfHJustification::Fit) {
        xAxisDir = gp_Vec{alignPnt1, alignPnt2};
        // Ensure non-null x-axis direction
        if (GeomUtils::isNull(xAxisDir))
            xAxisDir = gp::DX();
    }
    else if (!MathUtils::fuzzyIsNull(text.rotationAngle) && GeomUtils::equal(xAxisDir, gp::DX())) {
        // If rotation angle is non-null and x-axis direction defaults to standard Ox then
        // set x-axis so it matches rotation angle
        const double angle = MathUtils::degreeToRadian(text.rotationAngle);
        const gp_Trsf trsf = GeomUtils::makeRotation(gp_Ax1{pnt, extDir}, angle);
        xAxisDir = gp::DX().Transformed(trsf);
    }

    xAxisDir.Normalize();

    std::string textStr = this->toUtf8(std::string{text.str});
    DxfReader::replaceTextControlCodes(&textStr);
    const auto occTextStr = string_conv<NCollection_String>(textStr);
    double xScaleWidth = text.relativeXScaleFactorWidth;
    if (hjust == DxfHJustification::Fit || hjust == DxfHJustification::Aligned) {
        const double width = computeStringWidth(occTextStr, brepFont);
        const double pntDist = alignPnt1.Distance(alignPnt2);
        const double scale = !MathUtils::fuzzyIsNull(width) ? pntDist / width : pntDist;
        if (hjust == DxfHJustification::Aligned)
            brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight * scale);
        else
            xScaleWidth = scale;
    }

    Font_BRepTextBuilder brepTextBuilder;
    brepFont.SetWidthScaling(float(xScaleWidth));
    const gp_Ax3 locText(pnt, extDir, xAxisDir);
    TopoDS_Shape shape = brepTextBuilder.Perform(brepFont, occTextStr, locText, hAlign, vAlign);
    if (xMirror)
        shape.Move(GeomUtils::makeMirror(gp_Ax2{pnt, locText.XDirection()}));

    if (yMirror)
        shape.Move(GeomUtils::makeMirror(gp_Ax2{pnt, locText.YDirection()}));

    return shape;
}

TopoDS_Shape DxfReader::ReaderImpl::createShape(const Dxf_ATTRIB& attrib)
{
    return this->createShape(static_cast<const Dxf_TEXT&>(attrib));
}

TopoDS_Shape DxfReader::ReaderImpl::makeFace(
        const Dxf_QuadBase& quad, const std::function<bool(int)>& isEdgeVisible
    )
{
    const gp_Pnt p1 = toOccPnt(quad.corner1);
    const gp_Pnt p2 = toOccPnt(quad.corner2);
    const gp_Pnt p3 = toOccPnt(quad.corner3);
    const gp_Pnt p4 = toOccPnt(quad.corner4);

    if (GeomUtils::equal(p1, p2) || GeomUtils::equal(p1, p3) || GeomUtils::equal(p2, p3))
        return {};

    // Get count of visible edges
    unsigned visibleCount = isEdgeVisible ? 0 : 4;
    if (isEdgeVisible) {
        for (int i = 1; i <= 4; ++i)
            visibleCount += isEdgeVisible(i) ? 1 : 0;
    }

    if (visibleCount == 0)
        return {};

    // Helper function to ease usage of isEdgeVisible()
    auto checkEdgeVisible = [&](int index) {
        return !isEdgeVisible || isEdgeVisible(index);
    };
    const bool isQuad = quad.hasCorner4 && !GeomUtils::equal(p3, p4) && !GeomUtils::equal(p1, p4);
    const bool isClosed = visibleCount == 4 || (!isQuad && visibleCount == 3 && !checkEdgeVisible(4));

    // Helper function to abstract addition of edges
    BRepBuilderAPI_MakeWire makeWire;
    TopoDS_Shape compound;
    auto addEdge = [&](int index, const gp_Pnt& startPnt, const gp_Pnt& endPnt) {
        if (checkEdgeVisible(index)) {
            const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(startPnt, endPnt);
            if (isClosed)
                makeWire.Add(edge);
            else
                BRepUtils::addShape(&compound, edge);
        }
    };

    addEdge(1, p1, p2);
    addEdge(2, p2, p3);
    if (isQuad) {
        addEdge(3, p3, p4);
        addEdge(4, p4, p1);
    }
    else {
        addEdge(3, p3, p1);
    }

    if (isClosed && makeWire.IsDone())
        return BRepBuilderAPI_MakeFace(makeWire.Wire(), true/*onlyPlane*/);

    return compound;
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
TopoDS_Shape DxfReader::ReaderImpl::createSplineFromPolesAndKnots(const Dxf_SPLINE& spline)
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
        if (occMults.Value(i) > spline.degree) { // required: <= degree
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

        BRepBuilderAPI_MakeWire makeWire;
        for (int i = 0; i < numSegments; ++i) {
            TColgp_Array1OfPnt segPoles(1, numPolesPerSegment);
            for (int j = 0; j < numPolesPerSegment; ++j)
                segPoles.SetValue(j+1, occPoles.Value(i * numPolesPerSegment + j + 1));

            makeWire.Add(BRepBuilderAPI_MakeEdge(makeOccHandle<Geom_BezierCurve>(segPoles)));
        }

        if (makeWire.IsDone())
            return makeWire.Wire();
    }

    return {};
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
TopoDS_Shape DxfReader::ReaderImpl::createInterpolationSpline(const Dxf_SPLINE& spline)
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
    if (!GeomUtils::isNull(tanStart) || !GeomUtils::isNull(tanEnd))
        interp.Load(tanStart, tanEnd, true/*scale*/);

    interp.Perform();
    if (!interp.IsDone())
        return {}; // TODO Emit error message

    return BRepBuilderAPI_MakeEdge(interp.Curve());
}

} // namespace Mayo::IO
