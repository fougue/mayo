/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
    std::unique_ptr<PropertyGroup> properties(gsl::span<const GraphicsObjectPtr> spanObject) const override;

    static Support pointCloudSupportStatus(const TDF_Label& label);

    DEFINE_STANDARD_RTTI_INLINE(GraphicsPointCloudObjectDriver, GraphicsObjectDriver)
};

} // namespace Mayo
