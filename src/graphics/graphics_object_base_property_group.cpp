/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_object_base_property_group.h"
#include "graphics_object_driver.h"
#include "graphics_utils.h"
#include "../base/application.h"
#include "../base/application_item_selection_model.h"
#include "../base/text_id.h"

namespace Mayo {

GraphicsObjectBasePropertyGroup::GraphicsObjectBasePropertyGroup(Span<const GraphicsObjectPtr> spanObject)
    : m_vecObject(spanObject.cbegin(), spanObject.cend())/*,
      m_propertyVisibleState(this, textId("visible"))*/
{
    // Init properties
    Mayo_PropertyChangedBlocker(this);
#if 0
    int visibleCount = 0;
    for (const GraphicsObjectPtr& object : spanObject) {
        if (GraphicsUtils::AisObject_isVisible(object))
            ++visibleCount;
    }

    if (visibleCount == 0)
        m_propertyVisibleState.setValue(Qt::Unchecked);
    else
        m_propertyVisibleState.setValue(visibleCount == spanObject.size() ? Qt::Checked : Qt::PartiallyChecked);
#endif
}

void GraphicsObjectBasePropertyGroup::onPropertyChanged(Property* prop)
{
#if 0
    if (prop == &m_propertyVisibleState) {
        if (m_propertyVisibleState != Qt::PartiallyChecked) {
            const bool isVisible = m_propertyVisibleState == Qt::Checked;
            for (const GraphicsObjectPtr& object : m_vecObject)
                GraphicsUtils::AisObject_setVisible(object, isVisible);

            emit this->visibilityToggled(isVisible);
        }
    }
#endif

    PropertyGroupSignals::onPropertyChanged(prop);
}

} // namespace Mayo
