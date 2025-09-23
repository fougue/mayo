/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_environment.h"

#include "../base/property_value_conversion.h"

namespace Mayo {

const PropertyValueConversion& ScriptEnvironment::getPropertyValueConverter(const ScriptEnvironment& env)
{
    static const PropertyValueConversion defaultPropValueConverter;
    return env.propertyValueConverter ? *env.propertyValueConverter : defaultPropValueConverter;
}

} // namespace Mayo
