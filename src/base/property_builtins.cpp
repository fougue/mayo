/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "property_builtins.h"

namespace Mayo {

BasePropertyQuantity::BasePropertyQuantity(PropertyGroup* grp, const TextId& name)
    : Property(grp, name)
{
}

template<> const char PropertyBool::TypeName[] = "Mayo::PropertyBool";
template<> const char GenericProperty<int>::TypeName[] = "Mayo::PropertyInt";
template<> const char GenericProperty<double>::TypeName[] = "Mayo::PropertyDouble";
template<> const char PropertyString::TypeName[] = "Mayo::PropertyString";
template<> const char GenericProperty<CheckState>::TypeName[] = "Mayo::PropertyCheckState";
template<> const char PropertyOccPnt::TypeName[] = "Mayo::PropertyOccPnt";
template<> const char PropertyOccVec::TypeName[] = "Mayo::PropertyOccVec";
template<> const char PropertyOccTrsf::TypeName[] = "Mayo::PropertyOccTrsf";
template<> const char GenericProperty<Quantity_Color>::TypeName[] = "Mayo::PropertyOccColor";
template<> const char GenericProperty<FilePath>::TypeName[] = "Mayo::PropertyFilePath";

} // namespace Mayo
