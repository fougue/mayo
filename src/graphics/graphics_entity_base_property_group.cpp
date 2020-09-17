/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_entity_base_property_group.h"
#include "graphics_entity_driver.h"

namespace Mayo {

namespace Internal {

static const Enumeration* displayModesEnum(const GraphicsEntity& gfxEntity) {
    const GraphicsEntityDriver* driverPtr = gfxEntity.driverPtr();
    return driverPtr ? &(driverPtr->displayModes()) : nullptr;
}

static Enumeration::Value currentDisplayMode(const GraphicsEntity& gfxEntity) {
    const GraphicsEntityDriver* driverPtr = gfxEntity.driverPtr();
    return driverPtr ? driverPtr->currentDisplayMode(gfxEntity) : -1;
}

} // namespace Internal

GraphicsEntityBasePropertyGroup::GraphicsEntityBasePropertyGroup(const GraphicsEntity& gfxEntity)
    : m_gfxEntity(gfxEntity),
      m_propertyIsVisible(this, tr("Visible")),
      m_propertyDisplayMode(this, tr("Display mode"), Internal::displayModesEnum(gfxEntity))
{
    // Init properties
    Mayo_PropertyChangedBlocker(this);

    m_propertyIsVisible.setValue(gfxEntity.isVisible());
    m_propertyDisplayMode.setValue(Internal::currentDisplayMode(gfxEntity));
}

void GraphicsEntityBasePropertyGroup::onPropertyChanged(Property *prop)
{
    if (prop == &m_propertyIsVisible)
        m_gfxEntity.setVisible(m_propertyIsVisible.value());
    else if (prop == &m_propertyDisplayMode)
        m_gfxEntity.driverPtr()->applyDisplayMode(m_gfxEntity, m_propertyDisplayMode.value());

    PropertyGroupSignals::onPropertyChanged(prop);
}

} // namespace Mayo
