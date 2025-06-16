/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_driver.h"

namespace Mayo {

// Pre-declarations
class GraphicsPointCloudObjectDriver;
DEFINE_STANDARD_HANDLE(GraphicsPointCloudObjectDriver, GraphicsObjectDriver)
using GraphicsPointCloudObjectDriverPtr = OccHandle<GraphicsPointCloudObjectDriver>;

// Provides creation and configuration of graphics objects for point clouds
class GraphicsPointCloudObjectDriver : public GraphicsObjectDriver {
public:
    GraphicsPointCloudObjectDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsObjectPtr createObject(const TDF_Label& label) const override;
    void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const override;
    Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const override;
    std::unique_ptr<PropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const override;

    static Support pointCloudSupportStatus(const TDF_Label& label);

    DEFINE_STANDARD_RTTI_INLINE(GraphicsPointCloudObjectDriver, GraphicsObjectDriver)
};

} // namespace Mayo
