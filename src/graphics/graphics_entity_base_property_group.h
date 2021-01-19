/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "graphics_entity.h"

namespace Mayo {

class GraphicsEntityBasePropertyGroup : public PropertyGroupSignals {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsEntityBasePropertyGroup)
public:
    GraphicsEntityBasePropertyGroup(const GraphicsEntity& gfxEntity);
    void onPropertyChanged(Property* prop) override;

private:
    GraphicsEntity m_gfxEntity;
    PropertyBool m_propertyIsVisible;
    PropertyEnumeration m_propertyDisplayMode;
};

} // namespace Mayo
