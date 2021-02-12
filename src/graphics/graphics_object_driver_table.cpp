/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_object_driver_table.h"
#include "graphics_object_driver.h"

namespace Mayo {

void GraphicsObjectDriverTable::addDriver(DriverPtr driver)
{
    m_vecDriver.push_back(driver);
}

void GraphicsObjectDriverTable::addDriver(std::unique_ptr<GraphicsObjectDriver> driver)
{
    m_vecDriver.push_back(driver.release());
}

GraphicsObjectPtr GraphicsObjectDriverTable::createObject(const TDF_Label& label) const
{
    GraphicsObjectDriver* driverPartialSupport = nullptr;
    for (const DriverPtr& driver : m_vecDriver) {
        const GraphicsObjectDriver::Support support = driver->supportStatus(label);
        if (support == GraphicsObjectDriver::Support::Complete)
            return driver->createObject(label);

        if (support == GraphicsObjectDriver::Support::Partial)
            driverPartialSupport = driver.get();
    }

    if (driverPartialSupport)
        return driverPartialSupport->createObject(label);
    else
        return {};
}

} // namespace Mayo
