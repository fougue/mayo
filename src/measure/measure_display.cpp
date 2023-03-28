/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "measure_display.h"

#include "../base/text_id.h"
#include "../base/unit_system.h"
#include "measure_tool.h"

#include <ElCLib.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Circle.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OpenGl_Context.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_TextAspect.hxx>

#include <fmt/format.h>

namespace Mayo {

namespace {
struct MeasureDisplayI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::MeasureDisplayI18N) };
} // namespace

std::unique_ptr<IMeasureDisplay> BaseMeasureDisplay::createFrom(MeasureType type, const MeasureValue& value)
{
    switch (type) {
    case MeasureType::VertexPosition:
        return std::make_unique<MeasureDisplayVertex>(std::get<gp_Pnt>(value));
    case MeasureType::CircleCenter:
        return std::make_unique<MeasureDisplayCircleCenter>(std::get<MeasureCircle>(value));
    case MeasureType::CircleDiameter:
        return std::make_unique<MeasureDisplayCircleDiameter>(std::get<MeasureCircle>(value));
    case MeasureType::MinDistance:
        return std::make_unique<MeasureDisplayMinDistance>(std::get<MeasureMinDistance>(value));
    case MeasureType::Length:
        return std::make_unique<MeasureDisplayLength>(std::get<QuantityLength>(value));
    case MeasureType::Angle:
        return std::make_unique<MeasureDisplayAngle>(std::get<MeasureAngle>(value));
    case MeasureType::Area:
        return std::make_unique<MeasureDisplayArea>(std::get<QuantityArea>(value));
    default:
        return {};
    }
}

std::unique_ptr<IMeasureDisplay> BaseMeasureDisplay::createEmptySumFrom(MeasureType type)
{
    switch (type) {
    case MeasureType::Length:
        return std::make_unique<MeasureDisplayLength>(Mayo::QuantityLength{0});
    case MeasureType::Area:
        return std::make_unique<MeasureDisplayArea>(Mayo::QuantityArea{0});
    default:
        return {};
    }
}

void BaseMeasureDisplay::adaptGraphics(const Handle_Graphic3d_GraphicDriver& driver)
{
    const auto openGlDriver = Handle_OpenGl_GraphicDriver::DownCast(driver);
    const auto openGlContext = openGlDriver ? openGlDriver->GetSharedContext() : nullptr;
    if (!openGlContext)
        return;

    const bool useVbo = openGlContext->ToUseVbo();
    for (int i = 0; i < this->graphicsObjectsCount(); ++i) {
        auto gfxText = Handle(AIS_TextLabel)::DownCast(this->graphicsObjectAt(i));
        if (gfxText) {
            // NOTE
            // Usage of Aspect_TODT_SUBTITLE is causing a crash when VBO are not available(eg because
            // of too old OpenGL version)
            gfxText->SetDisplayType(useVbo ? Aspect_TODT_SUBTITLE : Aspect_TODT_NORMAL);
            gfxText->SetColor(useVbo ? Quantity_NOC_WHITE : Quantity_NOC_BLACK);
            gfxText->SetTransparency(useVbo ? 0.2 : 0.);
        }
    }
}

void BaseMeasureDisplay::sumAdd(const IMeasureDisplay& /*other*/)
{
    ++m_sumCount;
}

std::string_view BaseMeasureDisplay::sumTextOr(std::string_view singleItemText) const
{
    return m_sumCount > 1 ? MeasureDisplayI18N::textIdTr("Sum") : singleItemText;
}

std::string BaseMeasureDisplay::text(const gp_Pnt& pnt, const MeasureDisplayConfig& config)
{
    auto trPntX = UnitSystem::translateLength(pnt.X() * Quantity_Millimeter, config.lengthUnit);
    auto trPntY = UnitSystem::translateLength(pnt.Y() * Quantity_Millimeter, config.lengthUnit);
    auto trPntZ = UnitSystem::translateLength(pnt.Z() * Quantity_Millimeter, config.lengthUnit);
    const std::string str = fmt::format(
                MeasureDisplayI18N::textIdTr("(<font color=\"#FF5500\">X</font>{0} "
                                             "<font color=\"#55FF00\">Y</font>{1} "
                                             "<font color=\"#0077FF\">Z</font>{2}){3}"),
                text(trPntX, config), text(trPntY, config), text(trPntZ, config),
                trPntX.strUnit
    );
    return str;
}

std::string BaseMeasureDisplay::text(double value, const MeasureDisplayConfig& config)
{
    return to_stdString(value, config.doubleToStringOptions);
}

std::string BaseMeasureDisplay::graphicsText(const gp_Pnt& pnt, const MeasureDisplayConfig& config)
{
    const std::string BaseMeasureDisplay = fmt::format(
                MeasureDisplayI18N::textIdTr(" X{0} Y{1} Z{2}"),
                text(UnitSystem::translateLength(pnt.X() * Quantity_Millimeter, config.lengthUnit), config),
                text(UnitSystem::translateLength(pnt.Y() * Quantity_Millimeter, config.lengthUnit), config),
                text(UnitSystem::translateLength(pnt.Z() * Quantity_Millimeter, config.lengthUnit), config)
    );
    return BaseMeasureDisplay;
}

void BaseMeasureDisplay::adaptScale(const Handle_AIS_TextLabel& gfxText, const MeasureDisplayConfig& config)
{
    static const Prs3d_TextAspect defaultTextAspect;
    gfxText->SetHeight(defaultTextAspect.Height() * config.devicePixelRatio);
}

void BaseMeasureDisplay::applyGraphicsDefaults(IMeasureDisplay* measureDisplay)
{
    for (int i = 0; i < measureDisplay->graphicsObjectsCount(); ++i) {
        auto gfxObject = measureDisplay->graphicsObjectAt(i);
        auto gfxText = Handle(AIS_TextLabel)::DownCast(gfxObject);
        if (gfxText) {
            gfxText->SetDisplayType(Aspect_TODT_SUBTITLE);
            gfxText->SetColorSubTitle(Quantity_NOC_BLACK);
            gfxText->SetColor(Quantity_NOC_WHITE);
            gfxText->SetTransparency(0.2);
        }
        else {
            gfxObject->SetColor(Quantity_NOC_BLACK);
        }
    }
}

// --
// -- Vertex
// --

MeasureDisplayVertex::MeasureDisplayVertex(const gp_Pnt& pnt)
    : m_pnt(pnt),
      m_gfxText(new AIS_TextLabel)
{
    m_gfxText->SetPosition(pnt);
    BaseMeasureDisplay::applyGraphicsDefaults(this);
}

void MeasureDisplayVertex::update(const MeasureDisplayConfig& config)
{
    this->setText(BaseMeasureDisplay::text(m_pnt, config));
    m_gfxText->SetText(to_OccExtString(BaseMeasureDisplay::graphicsText(m_pnt, config)));
    BaseMeasureDisplay::adaptScale(m_gfxText, config);
}

GraphicsObjectPtr MeasureDisplayVertex::graphicsObjectAt(int i) const
{
    return i == 0 ? m_gfxText : GraphicsObjectPtr{};
}

// --
// -- CircleCenter
// --

MeasureDisplayCircleCenter::MeasureDisplayCircleCenter(const MeasureCircle& circle)
    : m_circle(circle.value),
      m_gfxPoint(new AIS_Point(new Geom_CartesianPoint(m_circle.Location()))),
      m_gfxText(new AIS_TextLabel)
{
    m_gfxText->SetPosition(m_circle.Location());
    if (circle.isArc)
        m_gfxCircle = new AIS_Circle(new Geom_Circle(m_circle));

    BaseMeasureDisplay::applyGraphicsDefaults(this);
    // BEWARE LineAspect() is created indirectly by SetColor() call in applyGraphicsDefaults() function
    if (m_gfxCircle)
        m_gfxCircle->Attributes()->LineAspect()->SetTypeOfLine(Aspect_TOL_DOT);
}

void MeasureDisplayCircleCenter::update(const MeasureDisplayConfig& config)
{
    this->setText(BaseMeasureDisplay::text(m_circle.Location(), config));
    m_gfxText->SetText(to_OccExtString(BaseMeasureDisplay::graphicsText(m_circle.Location(), config)));
    BaseMeasureDisplay::adaptScale(m_gfxText, config);
}

int MeasureDisplayCircleCenter::graphicsObjectsCount() const
{
    return m_gfxCircle ? 3 : 2;
}

GraphicsObjectPtr MeasureDisplayCircleCenter::graphicsObjectAt(int i) const
{
    switch (i) {
    case 0: return m_gfxPoint;
    case 1: return m_gfxText;
    case 2: return m_gfxCircle;
    }
    return GraphicsObjectPtr{};
}

// --
// -- CircleDiameter
// --

MeasureDisplayCircleDiameter::MeasureDisplayCircleDiameter(const MeasureCircle& circle)
    : m_circle(circle.value),
      m_gfxCircle(new AIS_Circle(new Geom_Circle(m_circle))),
      m_gfxDiameterText(new AIS_TextLabel)
{
    const gp_Pnt otherPntAnchor = diameterOpposedPnt(circle.pntAnchor, m_circle);
    m_gfxDiameter = new AIS_Line(new Geom_CartesianPoint(circle.pntAnchor), new Geom_CartesianPoint(otherPntAnchor));
    m_gfxDiameter->SetWidth(2.5);
    m_gfxDiameter->Attributes()->LineAspect()->SetTypeOfLine(Aspect_TOL_DOT);

    m_gfxDiameterText->SetPosition(m_circle.Location());

    BaseMeasureDisplay::applyGraphicsDefaults(this);
    // BEWARE LineAspect() is created indirectly by SetColor() call in applyGraphicsDefaults() function
    m_gfxCircle->Attributes()->LineAspect()->SetTypeOfLine(Aspect_TOL_DOT);
}

void MeasureDisplayCircleDiameter::update(const MeasureDisplayConfig& config)
{
    const QuantityLength diameter = 2 * m_circle.Radius() * Quantity_Millimeter;
    const auto trDiameter = UnitSystem::translateLength(diameter, config.lengthUnit);
    const auto strDiameter = BaseMeasureDisplay::text(trDiameter, config);
    this->setText(fmt::format(
                      MeasureDisplayI18N::textIdTr("Diameter: {0}{1}"), strDiameter, trDiameter.strUnit
    ));

    m_gfxDiameterText->SetText(to_OccExtString(fmt::format(MeasureDisplayI18N::textIdTr(" Ø{0}"), strDiameter)));
    BaseMeasureDisplay::adaptScale(m_gfxDiameterText, config);
}

GraphicsObjectPtr MeasureDisplayCircleDiameter::graphicsObjectAt(int i) const
{
    switch (i) {
    case 0: return m_gfxCircle;
    case 1: return m_gfxDiameter;
    case 2: return m_gfxDiameterText;
    }

    return {};
}

gp_Pnt MeasureDisplayCircleDiameter::diameterOpposedPnt(const gp_Pnt& pntOnCircle, const gp_Circ& circ)
{
    return pntOnCircle.Translated(2 * gp_Vec{ pntOnCircle, circ.Location() });
}

// --
// -- MinDistance
// --

MeasureDisplayMinDistance::MeasureDisplayMinDistance(const MeasureMinDistance& dist)
    : m_dist(dist),
      m_gfxLength(new AIS_Line(new Geom_CartesianPoint(dist.pnt1), new Geom_CartesianPoint(dist.pnt2))),
      m_gfxDistText(new AIS_TextLabel),
      m_gfxPnt1(new AIS_Point(new Geom_CartesianPoint(dist.pnt1))),
      m_gfxPnt2(new AIS_Point(new Geom_CartesianPoint(dist.pnt2)))
{
    m_gfxLength->SetWidth(2.5);
    m_gfxLength->Attributes()->LineAspect()->SetTypeOfLine(Aspect_TOL_DOT);
    m_gfxDistText->SetPosition(dist.pnt1.Translated(gp_Vec(dist.pnt1, dist.pnt2) / 2.));
    BaseMeasureDisplay::applyGraphicsDefaults(this);
}

void MeasureDisplayMinDistance::update(const MeasureDisplayConfig& config)
{
    const auto trLength = UnitSystem::translateLength(m_dist.value, config.lengthUnit);
    const auto strLength = BaseMeasureDisplay::text(trLength, config);
    this->setText(fmt::format(
                      MeasureDisplayI18N::textIdTr("Min Distance: {0}{1}<br>Point1: {2}<br>Point2: {3}"),
                      strLength,
                      trLength.strUnit,
                      BaseMeasureDisplay::text(m_dist.pnt1, config),
                      BaseMeasureDisplay::text(m_dist.pnt2, config)
    ));
    m_gfxDistText->SetText(to_OccExtString(" " + strLength));
    BaseMeasureDisplay::adaptScale(m_gfxDistText, config);
}

GraphicsObjectPtr MeasureDisplayMinDistance::graphicsObjectAt(int i) const
{
    switch (i) {
    case 0: return m_gfxLength;
    case 1: return m_gfxDistText;
    case 2: return m_gfxPnt1;
    case 3: return m_gfxPnt2;
    }

    return {};
}

// --
// -- Angle
// --

MeasureDisplayAngle::MeasureDisplayAngle(MeasureAngle angle)
    : m_angle(angle),
      m_gfxEntity1(new AIS_Line(new Geom_CartesianPoint(angle.pntCenter), new Geom_CartesianPoint(angle.pnt1))),
      m_gfxEntity2(new AIS_Line(new Geom_CartesianPoint(angle.pntCenter), new Geom_CartesianPoint(angle.pnt2))),
      m_gfxAngleText(new AIS_TextLabel)
{
    const gp_Vec vec1(angle.pntCenter, angle.pnt1);
    const gp_Vec vec2(angle.pntCenter, angle.pnt2);
    const gp_Ax2 axCircle(angle.pntCenter, vec1.Crossed(vec2), vec1);
    Handle_Geom_Circle geomCircle = new Geom_Circle(axCircle, 0.8 * vec1.Magnitude());
    const double param1 = ElCLib::Parameter(geomCircle->Circ(), angle.pnt1);
    const double param2 = ElCLib::Parameter(geomCircle->Circ(), angle.pnt2);
    m_gfxAngle = new AIS_Circle(geomCircle, param1, param2);
    BaseMeasureDisplay::applyGraphicsDefaults(this);
    m_gfxAngle->SetWidth(2);
    m_gfxEntity1->Attributes()->LineAspect()->SetTypeOfLine(Aspect_TOL_DOT);
    m_gfxEntity2->Attributes()->LineAspect()->SetTypeOfLine(Aspect_TOL_DOT);
    m_gfxAngleText->SetPosition(ElCLib::Value(param1 + std::abs((param2 - param1) / 2.), geomCircle->Circ()));
}

void MeasureDisplayAngle::update(const MeasureDisplayConfig& config)
{
    const auto trAngle = UnitSystem::translateAngle(m_angle.value, config.angleUnit);
    const auto strAngle = BaseMeasureDisplay::text(trAngle, config);
    this->setText(fmt::format(MeasureDisplayI18N::textIdTr("Angle: {0}{1}"), strAngle, trAngle.strUnit));
    m_gfxAngleText->SetText(to_OccExtString(" " + strAngle));
    BaseMeasureDisplay::adaptScale(m_gfxAngleText, config);
}

int MeasureDisplayAngle::graphicsObjectsCount() const
{
    return 4;
}

GraphicsObjectPtr MeasureDisplayAngle::graphicsObjectAt(int i) const
{
    switch (i) {
    case 0: return m_gfxAngle;
    case 1: return m_gfxEntity1;
    case 2: return m_gfxEntity2;
    case 3: return m_gfxAngleText;
    }

    return {};
}

// --
// -- Length
// --

MeasureDisplayLength::MeasureDisplayLength(QuantityLength length)
    : m_length(length)
{
}

void MeasureDisplayLength::update(const MeasureDisplayConfig& config)
{
    const auto trLength = UnitSystem::translateLength(m_length, config.lengthUnit);
    this->setText(fmt::format(
                      MeasureDisplayI18N::textIdTr("{0}: {1}{2}"),
                      BaseMeasureDisplay::sumTextOr(MeasureDisplayI18N::textIdTr("Length")),
                      BaseMeasureDisplay::text(trLength, config),
                      trLength.strUnit
    ));
}

void MeasureDisplayLength::sumAdd(const IMeasureDisplay& other)
{
    const auto& otherLen = dynamic_cast<const MeasureDisplayLength&>(other);
    m_length += otherLen.m_length;
    BaseMeasureDisplay::sumAdd(other);
}

// --
// -- Area
// --

MeasureDisplayArea::MeasureDisplayArea(QuantityArea area)
    : m_area(area)
{
}

void MeasureDisplayArea::update(const MeasureDisplayConfig& config)
{
    const auto trArea = UnitSystem::translateArea(m_area, config.areaUnit);
    this->setText(fmt::format(
                      MeasureDisplayI18N::textIdTr("{0}: {1}{2}"),
                      BaseMeasureDisplay::sumTextOr(MeasureDisplayI18N::textIdTr("Area")),
                      BaseMeasureDisplay::text(trArea, config),
                      trArea.strUnit
    ));
}

void MeasureDisplayArea::sumAdd(const IMeasureDisplay& other)
{
    const auto& otherArea = dynamic_cast<const MeasureDisplayArea&>(other);
    m_area += otherArea.m_area;
    BaseMeasureDisplay::sumAdd(other);
}

} // namespace Mayo
