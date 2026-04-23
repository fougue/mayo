/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "graphics_object_driver.h"

namespace Mayo {

GraphicsObjectDriverPtr GraphicsObjectDriver::get(const GraphicsObjectPtr& object)
{
    if (object)
        return GraphicsObjectDriverPtr::DownCast(object->GetOwner());
    else
        return {};
}

GraphicsObjectDriverPtr GraphicsObjectDriver::getCommon(gsl::span<const GraphicsObjectPtr> spanObject)
{
    GraphicsObjectDriverPtr commonGfxDriver;
    for (const GraphicsObjectPtr& object : spanObject) {
        GraphicsObjectDriverPtr gfxDriver = GraphicsObjectDriver::get(object);
        if (!commonGfxDriver)
            commonGfxDriver = gfxDriver;
        else if (commonGfxDriver != gfxDriver)
            return {};
    }

    return commonGfxDriver;
}

void GraphicsObjectDriver::throwIf_invalidDisplayMode(Enumeration::Value mode) const
{
    if (this->displayModes().findIndexByValue(mode) == -1)
        throw std::invalid_argument("Invalid display mode");
}

void GraphicsObjectDriver::throwIf_differentDriver(const GraphicsObjectPtr& object) const
{
    if (GraphicsObjectDriver::get(object) != this)
        throw std::invalid_argument("Invalid driver for graphics object");
}

void GraphicsObjectDriver::throwIf_differentDriver(gsl::span<const GraphicsObjectPtr> objects) const
{
    for (const GraphicsObjectPtr& object : objects)
        this->throwIf_differentDriver(object);
}

} // namespace Mayo
