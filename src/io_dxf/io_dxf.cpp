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
#include "../base/messenger.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/string_conv.h"
#include "../base/tkernel_utils.h"
#include "../base/unit_system.h"
#include "aci_table.h"
#include "dxf.h"

#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Trsf.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <Graphic3d_VerticalTextAlignment.hxx>
#include <Font_BRepTextBuilder.hxx>
#include <Font_FontMgr.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Precision.hxx>
#include <Resource_Unicode.hxx>
#include <TDataStd_Name.hxx>
#include <TopoDS_Edge.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <fmt/format.h>
#include <optional>
#include <sstream>
#include <string_view>

namespace Mayo {
namespace IO {

namespace {

bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}

const Enumeration& systemFontNames()
{
    static Enumeration fontNames;
    static TColStd_SequenceOfHAsciiString seqFontName;
    if (fontNames.empty()) {
        Handle_Font_FontMgr fontMgr = Font_FontMgr::GetInstance();
        fontMgr->GetAvailableFontsNames(seqFontName);
        int i = 0;
        for (const Handle_TCollection_HAsciiString& fontName : seqFontName)
            fontNames.addItem(i++, { {}, to_stdStringView(fontName->String()) });
    }

    return fontNames;
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
    void OnReadLine(const double* s, const double* e, bool hidden) override;
    void OnReadPolyline(const DxfPolyline& polyline) override;
    void OnReadPoint(const double* s) override;
    void OnReadText(const DxfText& text) override;
    void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool hidden) override;
    void OnReadCircle(const double* s, const double* c, bool dir, bool hidden) override;
    void OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir) override;
    void OnReadSpline(struct SplineData& sd) override;
    void OnReadInsert(const double* point, const double* scale, const char* name, double rotation) override;
    void OnReadDimension(const double* s, const double* e, const double* point, double rotation) override;

    void ReportError(const std::string& msg) override;

    static Handle_Geom_BSplineCurve createSplineFromPolesAndKnots(struct SplineData& sd);
    static Handle_Geom_BSplineCurve createInterpolationSpline(struct SplineData& sd);

    gp_Pnt toPnt(const double* coords) const;
    void addShape(const TopoDS_Shape& shape);
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
    Handle_XCAFDoc_ShapeTool shapeTool = doc->xcaf().shapeTool();
    Handle_XCAFDoc_ColorTool colorTool = doc->xcaf().colorTool();
    Handle_XCAFDoc_LayerTool layerTool = doc->xcaf().layerTool();
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
        m_params.fontNameForTextObjects = ptr->fontNameForTextObjects.name();
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

void DxfReader::Internal::OnReadLine(const double* s, const double* e, bool /*hidden*/)
{
    const gp_Pnt p0 = this->toPnt(s);
    const gp_Pnt p1 = this->toPnt(e);
    if (p0.IsEqual(p1, Precision::Confusion()))
        return;

    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p0, p1);
    this->addShape(edge);
}

void DxfReader::Internal::OnReadPolyline(const DxfPolyline& polyline)
{
    const auto& vertices = polyline.vertices;
    Handle(Poly_Polygon3D) polygon = new Poly_Polygon3D(
        CppUtils::safeStaticCast<int>(vertices.size()),
        false/*!hasParams*/
    );
    for (unsigned i = 0; i < vertices.size(); ++i)
        polygon->ChangeNodes().ChangeValue(i + 1) = this->toPnt(vertices.at(i).point);

    this->addShape(BRepUtils::makeEdge(polygon));
}

void DxfReader::Internal::OnReadPoint(const double* s)
{
    const TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(this->toPnt(s));
    this->addShape(vertex);
}

void DxfReader::Internal::OnReadText(const DxfText& text)
{
    if (!m_params.importAnnotations)
        return;

    const gp_Pnt pt = this->toPnt(text.point);
    const std::string layerName = this->LayerName();
    if (!startsWith(layerName, "BLOCKS")) {
        const std::string& fontName = m_params.fontNameForTextObjects;
        const double fontHeight = 4 * text.height * m_params.scaling;
        Font_BRepFont brepFont;
        if (brepFont.Init(fontName.c_str(), Font_FA_Regular, fontHeight)) {
            gp_Trsf rotTrsf;
            if (!MathUtils::fuzzyIsNull(text.rotation))
                rotTrsf.SetRotation(gp_Ax1(pt, gp::DZ()), text.rotation);

            const int ap = static_cast<int>(text.attachPoint);
            Graphic3d_HorizontalTextAlignment hAttachPnt = Graphic3d_HTA_LEFT;
            if (ap == 2 || ap == 5 || ap == 8)
                hAttachPnt = Graphic3d_HTA_CENTER;
            else if (ap == 3 || ap == 6 || ap == 9)
                hAttachPnt = Graphic3d_HTA_RIGHT;

            Graphic3d_VerticalTextAlignment vAttachPnt = Graphic3d_VTA_TOP;
            if (ap == 4 || ap == 5 || ap == 6)
                vAttachPnt = Graphic3d_VTA_CENTER;
            else if (ap == 7 || ap == 8 || ap == 9)
                vAttachPnt = Graphic3d_VTA_BOTTOM;

            const gp_Ax3 locText(pt, gp::DZ(), gp::DX().Transformed(rotTrsf));
            Font_BRepTextBuilder brepTextBuilder;
            const TopoDS_Shape shapeText = brepTextBuilder.Perform(
                brepFont, string_conv<NCollection_String>(text.str), locText, hAttachPnt, vAttachPnt
            );
            this->addShape(shapeText);
        }
        else {
            m_messenger->emitWarning(fmt::format("Font_BRepFont is null for '{}'", fontName));
        }
    }
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadArc(const double* s, const double* e, const double* c, bool dir, bool /*hidden*/)
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
void DxfReader::Internal::OnReadCircle(const double* s, const double* c, bool dir, bool /*hidden*/)
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
        const double* c,
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
void DxfReader::Internal::OnReadSpline(SplineData& sd)
{
    // https://documentation.help/AutoCAD-DXF/WS1a9193826455f5ff18cb41610ec0a2e719-79e1.htm
    // Flags:
    // 1: Closed, 2: Periodic, 4: Rational, 8: Planar, 16: Linear

    try {
        Handle_Geom_BSplineCurve geom;
        if (sd.control_points > 0)
            geom = createSplineFromPolesAndKnots(sd);
        else if (sd.fit_points > 0)
            geom = createInterpolationSpline(sd);

        if (geom.IsNull())
            throw Standard_Failure();

        const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(geom);
        this->addShape(edge);
    }
    catch (const Standard_Failure&) {
        m_messenger->emitWarning("DxfReader - Failed to create bspline");
    }
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
void DxfReader::Internal::OnReadInsert(const double* point, const double* scale, const char* name, double rotation)
{
    //std::cout << "Inserting block " << name << " rotation " << rotation << " pos " << point[0] << "," << point[1] << "," << point[2] << " scale " << scale[0] << "," << scale[1] << "," << scale[2] << std::endl;
    const std::string prefix = std::string("BLOCKS ") + name + " ";
    for (const auto& [k, vecEntity] : m_layers) {
        if (!startsWith(k, prefix))
            continue; // Skip

        TopoDS_Compound comp = BRepUtils::makeEmptyCompound();
        for (const DxfReader::Entity& entity : vecEntity) {
            if (!entity.shape.IsNull())
                BRepUtils::addShape(&comp, entity.shape);
        }

        if (comp.IsNull())
            continue; // Skip

        auto nonNull = [](double v) { return !MathUtils::fuzzyIsNull(v) ? v : 1.; };
        const double nscale[] = { nonNull(scale[0]), nonNull(scale[1]), nonNull(scale[2]) };
        gp_Trsf trsfScale;
        trsfScale.SetValues(
                nscale[0], 0,         0,         0,
                0,         nscale[1], 0,         0,
                0,         0,         nscale[2], 0
        );
        gp_Trsf trsfRotZ;
        trsfRotZ.SetRotation(gp::OZ(), rotation);
        gp_Trsf trsfMove;
        trsfMove.SetTranslation(this->toPnt(point).XYZ());
        const gp_Trsf trsf = trsfScale * trsfRotZ * trsfMove;
        comp.Location(trsf);
        this->addShape(comp);
    }
}

void DxfReader::Internal::OnReadDimension(const double* s, const double* e, const double* point, double rotation)
{
    if (m_params.importAnnotations) {
        // TODO
        std::stringstream sstr;
        sstr << "DxfReader::OnReadDimension() - Not yet implemented" << std::endl
             << "    s: " << s[0] << ", " << s[1] << ", " << s[2] << std::endl
             << "    e: " << e[0] << ", " << e[1] << ", " << e[2] << std::endl
             << "    point: " << point[0] << ", " << point[1] << ", " << point[2] << std::endl
             << "    rotation: " << rotation << std::endl;
        m_messenger->emitWarning(sstr.str());
    }
}

void DxfReader::Internal::ReportError(const std::string& msg)
{
    m_messenger->emitError(msg);
}

gp_Pnt DxfReader::Internal::toPnt(const double* coords) const
{
    double sp1(coords[0]);
    double sp2(coords[1]);
    double sp3(coords[2]);
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

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
Handle_Geom_BSplineCurve DxfReader::Internal::createSplineFromPolesAndKnots(struct SplineData& sd)
{
    const size_t numPoles = sd.control_points;
    if (sd.controlx.size() > numPoles
            || sd.controly.size() > numPoles
            || sd.controlz.size() > numPoles
            || sd.weight.size() > numPoles)
    {
        return {};
    }

    // handle the poles
    TColgp_Array1OfPnt occpoles(1, sd.control_points);
    int index = 1;
    for (auto x : sd.controlx)
        occpoles(index++).SetX(x);

    index = 1;
    for (auto y : sd.controly)
        occpoles(index++).SetY(y);

    index = 1;
    for (auto z : sd.controlz)
        occpoles(index++).SetZ(z);

    // handle knots and mults
    std::set<double> unique;
    unique.insert(sd.knot.begin(), sd.knot.end());

    const int numKnots = int(unique.size());
    TColStd_Array1OfInteger occmults(1, numKnots);
    TColStd_Array1OfReal occknots(1, numKnots);
    index = 1;
    for (auto k : unique) {
        const auto m = CppUtils::safeStaticCast<int>(std::count(sd.knot.begin(), sd.knot.end(), k));
        occknots(index) = k;
        occmults(index) = m;
        index++;
    }

    // handle weights
    TColStd_Array1OfReal occweights(1, sd.control_points);
    if (sd.weight.size() == size_t(sd.control_points)) {
        index = 1;
        for (auto w : sd.weight)
            occweights(index++) = w;
    }
    else {
        // non-rational
        for (int i = occweights.Lower(); i <= occweights.Upper(); i++)
            occweights(i) = 1.0;
    }

    const bool periodic = sd.flag == 2;
    return new Geom_BSplineCurve(occpoles, occweights, occknots, occmults, sd.degree, periodic);
}

// Excerpted from FreeCad/src/Mod/Import/App/ImpExpDxf
Handle_Geom_BSplineCurve DxfReader::Internal::createInterpolationSpline(struct SplineData& sd)
{
    const size_t numPoints = sd.fit_points;
    if (sd.fitx.size() > numPoints || sd.fity.size() > numPoints || sd.fitz.size() > numPoints)
        return {};

    // handle the poles
    Handle_TColgp_HArray1OfPnt fitpoints = new TColgp_HArray1OfPnt(1, sd.fit_points);
    int index = 1;
    for (auto x : sd.fitx)
        fitpoints->ChangeValue(index++).SetX(x);

    index = 1;
    for (auto y : sd.fity)
        fitpoints->ChangeValue(index++).SetY(y);

    index = 1;
    for (auto z : sd.fitz)
        fitpoints->ChangeValue(index++).SetZ(z);

    const bool periodic = sd.flag == 2;
    GeomAPI_Interpolate interp(fitpoints, periodic, Precision::Confusion());
    interp.Perform();
    return interp.Curve();
}

} // namespace IO
} // namespace Mayo
