/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "measure_tool.h"
#include "../graphics/graphics_object_ptr.h"

#include <AIS_Circle.hxx>
#include <AIS_Line.hxx>
#include <AIS_Point.hxx>
#include <AIS_TextLabel.hxx>
#include <gp_Circ.hxx>
#include <Quantity_Color.hxx>

#include <memory>
#include <string>
#include <string_view>

namespace Mayo {

struct MeasureConfig {
    LengthUnit lengthUnit = LengthUnit::Millimeter;
    AngleUnit angleUnit = AngleUnit::Degree;
    AreaUnit areaUnit = AreaUnit::SquareMillimeter;
};

// Provides an interface to textual/graphics representation of a measure
class IMeasureDisplay {
public:
    virtual ~IMeasureDisplay() = default;
    virtual void update(const MeasureConfig& config) = 0;
    virtual std::string text() const = 0;
    virtual int graphicsObjectsCount() const = 0;
    virtual GraphicsObjectPtr graphicsObjectAt(int i) const = 0;
    virtual bool isSumSupported() const = 0;
    virtual void sumAdd(const IMeasureDisplay& other) = 0;
};

// Base class for IMeasureDisplay implementations
class BaseMeasureDisplay : public IMeasureDisplay {
public:
    std::string text() const override { return m_text; }

    // Factory method to create an IMeasureDisplay object suited to input measure value
    static std::unique_ptr<IMeasureDisplay> createFrom(MeasureType type, const MeasureValue& value);
    static std::unique_ptr<IMeasureDisplay> createEmptySumFrom(MeasureType type);

    bool isSumSupported() const override { return false; }
    void sumAdd(const IMeasureDisplay& other) override;

protected:
    void setText(std::string_view str) { m_text = str; }

    int sumCount() const { return m_sumCount; }
    std::string_view sumTextOr(std::string_view singleItemText) const;

    static std::string text(const gp_Pnt& pnt, const MeasureConfig& config);
    static std::string text(double value);
    static std::string graphicsText(const gp_Pnt& pnt, const MeasureConfig& config);

    static void applyGraphicsDefaults(IMeasureDisplay* measureDisplay);

private:
    std::string m_text;
    int m_sumCount = 0;
};

// --
// -- Vertex
// --

class MeasureDisplayVertex : public BaseMeasureDisplay {
public:
    MeasureDisplayVertex(const gp_Pnt& pnt);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override { return 1; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    gp_Pnt m_pnt;
    Handle_AIS_TextLabel m_gfxText;
};

// --
// -- CircleCenter
// --

class MeasureDisplayCircleCenter : public BaseMeasureDisplay {
public:
    MeasureDisplayCircleCenter(const MeasureCircle& circle);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override;
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    gp_Circ m_circle;
    Handle_AIS_Point m_gfxPoint;
    Handle_AIS_TextLabel m_gfxText;
    Handle_AIS_Circle m_gfxCircle;
};

// --
// -- CircleDiameter
// --

class MeasureDisplayCircleDiameter : public BaseMeasureDisplay {
public:
    MeasureDisplayCircleDiameter(const MeasureCircle& circle);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override { return 3; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    static gp_Pnt diameterOpposedPnt(const gp_Pnt& pntOnCircle, const gp_Circ& circ);

    gp_Circ m_circle;
    Handle_AIS_Circle m_gfxCircle;
    Handle_AIS_Line m_gfxDiameter;
    Handle_AIS_TextLabel m_gfxDiameterText;
};

// --
// -- MinDistance
// --

class MeasureDisplayMinDistance : public BaseMeasureDisplay {
public:
    MeasureDisplayMinDistance(const MeasureMinDistance& dist);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override { return 4; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    MeasureMinDistance m_dist;
    Handle_AIS_Line m_gfxLength;
    Handle_AIS_TextLabel m_gfxDistText;
    Handle_AIS_Point m_gfxPnt1;
    Handle_AIS_Point m_gfxPnt2;
};

// --
// -- Angle
// --

class MeasureDisplayAngle : public BaseMeasureDisplay {
public:
    MeasureDisplayAngle(MeasureAngle angle);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override;
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    MeasureAngle m_angle;
    Handle_AIS_Line m_gfxEntity1;
    Handle_AIS_Line m_gfxEntity2;
    Handle_AIS_Circle m_gfxAngle;
    Handle_AIS_TextLabel m_gfxAngleText;
};

// --
// -- Length
// --

class MeasureDisplayLength : public BaseMeasureDisplay {
public:
    MeasureDisplayLength(QuantityLength length);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override { return 0; }
    GraphicsObjectPtr graphicsObjectAt(int /*i*/) const override { return {}; }

    bool isSumSupported() const override { return true; }
    void sumAdd(const IMeasureDisplay& other) override;

private:
    QuantityLength m_length;
};

// --
// -- Area
// --

class MeasureDisplayArea : public BaseMeasureDisplay {
public:
    MeasureDisplayArea(QuantityArea area);
    void update(const MeasureConfig& config) override;
    int graphicsObjectsCount() const override { return 0; }
    GraphicsObjectPtr graphicsObjectAt(int /*i*/) const override { return {}; }

    bool isSumSupported() const override { return true; }
    void sumAdd(const IMeasureDisplay& other) override;

private:
    QuantityArea m_area;
};

} // namespace Mayo
