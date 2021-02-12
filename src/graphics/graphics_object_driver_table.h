/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_driver.h"
#include "../base/span.h"
#include <memory>
#include <vector>

namespace Mayo {

class GraphicsObjectDriverTable {
public:
    using DriverPtr = GraphicsObjectDriverPtr;

    void addDriver(DriverPtr driver);
    void addDriver(std::unique_ptr<GraphicsObjectDriver> driver);
    Span<const DriverPtr> drivers() const { return m_vecDriver; }

    GraphicsObjectPtr createObject(const TDF_Label& label) const;

private:
    std::vector<DriverPtr> m_vecDriver;
};

} // namespace Mayo
