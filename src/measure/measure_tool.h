/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "measure_type.h"
#include "../base/quantity.h"
#include "../base/span.h"
#include "../graphics/graphics_object_ptr.h"
#include "../graphics/graphics_owner_ptr.h"

#include <gp_Circ.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>

#include <string_view>
#include <variant>

namespace Mayo {

// Void measure value
struct MeasureNone {};

// Measure of a distance between two entities
struct MeasureDistance {
    enum class Type {
        None, Mininmum, CenterToCenter
    };

    // Point on 1st entity from which the distance is measured
    gp_Pnt pnt1;
    // Point on 2nd entity from which the distance is measured
    gp_Pnt pnt2;
    // Length of the distance
    QuantityLength value;
    // Distance type
    Type type = Type::None;
};

// Measure of a circle entity
struct MeasureCircle {
    // "Start" point of the circle
    gp_Pnt pntAnchor;
    // Whether entity is a portion of circle(arc)
    bool isArc = false;
    // Circle definition
    gp_Circ value;
};

// Measure of the angle between two linear entities
struct MeasureAngle {
    // Point on the 1st entity forming the angle
    gp_Pnt pnt1;
    // Point on the 2nd entity forming the angle
    gp_Pnt pnt2;
    // Origin of the angle
    gp_Pnt pntCenter;
    // Value of the angle
    QuantityAngle value;
};

// Measure of the length of an entity
struct MeasureLength {
    // Point being at the middle of the entity
    gp_Pnt middlePnt;
    // Value of the length
    QuantityLength value;
};

// Measure of the area of an entity
struct MeasureArea {
    // Point being at the middle of the entity
    gp_Pnt middlePnt;
    // Value of the area
    QuantityArea value;
};

// Measure of the bounding box of an entity
struct MeasureBoundingBox {
    // Lower corner of the bounding box
    gp_Pnt cornerMin;
    // Upper corner of the bounding box
    gp_Pnt cornerMax;
    // Length of the bounding box along the X-axis
    QuantityLength xLength;
    // Length of the bounding box along the Y-axis
    QuantityLength yLength;
    // Length of the bounding box along the Z-axis
    QuantityLength zLength;
    // Volume of the bounding box
    QuantityVolume volume;
};

// Provides an interface to various measurement services
// Input data of a measure service is one or many graphics entities pointed to by GraphicsOwner objects
class IMeasureTool {
public:
    virtual ~IMeasureTool() = default;

    virtual Span<const GraphicsObjectSelectionMode> selectionModes(MeasureType type) const = 0;
    virtual bool supports(const GraphicsObjectPtr& object) const = 0;
    virtual bool supports(MeasureType type) const = 0;

    virtual gp_Pnt vertexPosition(const GraphicsOwnerPtr& owner) const = 0;
    virtual MeasureCircle circle(const GraphicsOwnerPtr& owner) const = 0;
    virtual MeasureDistance minDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const = 0;
    virtual MeasureDistance centerDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const = 0;
    virtual MeasureAngle angle(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const = 0;
    virtual MeasureLength length(const GraphicsOwnerPtr& owner) const = 0;
    virtual MeasureArea area(const GraphicsOwnerPtr& owner) const = 0;
    virtual MeasureBoundingBox boundingBox(const GraphicsOwnerPtr& owner) const = 0;
};

// Base interface for errors reported by measurement services of IMeasureTool
class IMeasureError {
public:
    virtual std::string_view message() const = 0;
};

// Union type for all the various measure values returned by IMeasureTool services
using MeasureValue = std::variant<
            MeasureNone, // WARNING: ensure this is the first value type in the variant
            gp_Pnt,
            MeasureCircle,
            MeasureDistance,
            MeasureAngle,
            MeasureLength,
            MeasureArea,
            MeasureBoundingBox
      >;

bool MeasureValue_isValid(const MeasureValue& res);

MeasureValue IMeasureTool_computeValue(
        const IMeasureTool& tool,
        MeasureType type,
        const GraphicsOwnerPtr& owner
);

MeasureValue IMeasureTool_computeValue(
        const IMeasureTool& tool,
        MeasureType type,
        const GraphicsOwnerPtr& owner1,
        const GraphicsOwnerPtr& owner2
);

} // namespace Mayo
