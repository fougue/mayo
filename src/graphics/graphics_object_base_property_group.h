/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/span.h"
#include "../base/property_builtins.h"
#include "graphics_object_ptr.h"
#include <vector>

namespace Mayo {

class GraphicsObjectBasePropertyGroup : public PropertyGroupSignals {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsObjectBasePropertyGroup)
public:
    GraphicsObjectBasePropertyGroup(Span<const GraphicsObjectPtr> spanObject);
    void onPropertyChanged(Property* prop) override;

//signals:
//    void visibilityToggled(bool on);

private:
    std::vector<GraphicsObjectPtr> m_vecObject;
    //PropertyCheckState m_propertyVisibleState;
};

} // namespace Mayo
