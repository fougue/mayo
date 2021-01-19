/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "graphics_object_ptr.h"

namespace Mayo {

class GraphicsObjectBasePropertyGroup : public PropertyGroupSignals {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsObjectBasePropertyGroup)
public:
    GraphicsObjectBasePropertyGroup(const GraphicsObjectPtr& object);
    void onPropertyChanged(Property* prop) override;

private:
    GraphicsObjectPtr m_object;
    PropertyBool m_propertyIsVisible;
    PropertyEnumeration m_propertyDisplayMode;
};

} // namespace Mayo
