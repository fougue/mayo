/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_driver.h"

namespace Mayo {

// Provides creation and configuration of graphics objects for BRep shapes
class GraphicsShapeObjectDriver : public GraphicsObjectDriver {
public:
    GraphicsShapeObjectDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsObjectPtr createObject(const TDF_Label& label) const override;
    void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const override;
    Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const override;
    std::unique_ptr<GraphicsObjectBasePropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const override;

    enum DisplayMode {
        DisplayMode_Wireframe,
        DisplayMode_HiddenLineRemoval,
        DisplayMode_Shaded,
        DisplayMode_ShadedWithFaceBoundary
    };
};

} // namespace Mayo
