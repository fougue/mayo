/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_driver.h"

namespace Mayo {

class GraphicsShapeObjectDriver;
DEFINE_STANDARD_HANDLE(GraphicsShapeObjectDriver, GraphicsObjectDriver)
using GraphicsShapeObjectDriverPtr = OccHandle<GraphicsShapeObjectDriver>;

// Provides creation and configuration of graphics objects for BRep shapes
class GraphicsShapeObjectDriver : public GraphicsObjectDriver {
public:
    GraphicsShapeObjectDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsObjectPtr createObject(const TDF_Label& label) const override;
    void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const override;
    Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const override;
    std::unique_ptr<PropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const override;

    static Support shapeSupportStatus(const TDF_Label& label);

    enum DisplayMode {
        DisplayMode_Wireframe,
        DisplayMode_HiddenLineRemoval,
        DisplayMode_Shaded,
        DisplayMode_ShadedWithFaceBoundary
    };

    DEFINE_STANDARD_RTTI_INLINE(GraphicsShapeObjectDriver, GraphicsObjectDriver)
};

} // namespace Mayo
