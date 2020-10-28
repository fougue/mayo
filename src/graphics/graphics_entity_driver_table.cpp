/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_entity_driver_table.h"
#include "graphics_entity_driver.h"

namespace Mayo {

void GraphicsEntityDriverTable::addDriver(DriverPtr driver)
{
    m_vecDriver.push_back(std::move(driver));
}

GraphicsEntity GraphicsEntityDriverTable::createEntity(const TDF_Label& label) const
{
    GraphicsEntityDriver* driverPartialSupport = nullptr;
    for (const DriverPtr& driver : m_vecDriver) {
        const GraphicsEntityDriver::Support support = driver->supportStatus(label);
        if (support == GraphicsEntityDriver::Support::Complete)
            return driver->createEntity(label);

        if (support == GraphicsEntityDriver::Support::Partial)
            driverPartialSupport = driver.get();
    }

    if (driverPartialSupport)
        return driverPartialSupport->createEntity(label);
    else
        return GraphicsEntity();
}

} // namespace Mayo
