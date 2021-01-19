/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_object_base_property_group.h"
#include "graphics_object_driver.h"
#include "graphics_utils.h"
#include "../base/text_id.h"

namespace Mayo {

namespace Internal {

static const Enumeration& displayModesEnum(const GraphicsObjectPtr& object) {
    auto driver = GraphicsObjectDriver::get(object);
    return driver ? driver->displayModes() : Enumeration::null();
}

static Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) {
    auto driver = GraphicsObjectDriver::get(object);
    return driver ? driver->currentDisplayMode(object) : -1;
}

} // namespace Internal

GraphicsObjectBasePropertyGroup::GraphicsObjectBasePropertyGroup(const GraphicsObjectPtr& object)
    : m_object(object),
      m_propertyIsVisible(this, textId("visible")),
      m_propertyDisplayMode(this, textId("displayMode"), Internal::displayModesEnum(object))
{
    // Init properties
    Mayo_PropertyChangedBlocker(this);
    m_propertyIsVisible.setValue(GraphicsUtils::AisObject_isVisible(object));
    m_propertyDisplayMode.setValue(Internal::currentDisplayMode(object));
}

void GraphicsObjectBasePropertyGroup::onPropertyChanged(Property* prop)
{
    if (prop == &m_propertyIsVisible) {
        GraphicsUtils::AisObject_setVisible(m_object, m_propertyIsVisible.value());
    }
    else if (prop == &m_propertyDisplayMode) {
        auto driver = GraphicsObjectDriver::get(m_object);
        driver->applyDisplayMode(m_object, m_propertyDisplayMode.value());
    }

    PropertyGroupSignals::onPropertyChanged(prop);
}

} // namespace Mayo
