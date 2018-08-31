/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_builtins.h"

namespace Mayo {

template<> const char PropertyBool::TypeName[] = "Mayo::PropertyBool";
template<> const char GenericProperty<int>/*PropertyInt*/::TypeName[] = "Mayo::PropertyInt";
template<> const char GenericProperty<double>/*PropertyDouble*/::TypeName[] = "Mayo::PropertyDouble";
template<> const char PropertyQByteArray::TypeName[] = "Mayo::PropertyQByteArray";
template<> const char PropertyQString::TypeName[] = "Mayo::PropertyQString";
template<> const char PropertyQDateTime::TypeName[] = "Mayo::PropertyQDateTime";
template<> const char PropertyOccColor::TypeName[] = "Mayo::PropertyOccColor";
template<> const char PropertyOccPnt::TypeName[] = "Mayo::PropertyOccPnt";
template<> const char PropertyOccTrsf::TypeName[] = "Mayo::PropertyOccTrsf";

template<> const char PropertyLength::TypeName[] = "Mayo::PropertyLength";
template<> const char PropertyArea::TypeName[] = "Mayo::PropertyArea";
template<> const char PropertyVolume::TypeName[] = "Mayo::PropertyVolume";
template<> const char PropertyMass::TypeName[] = "Mayo::PropertyMass";
template<> const char PropertyTime::TypeName[] = "Mayo::PropertyTime";
template<> const char PropertyAngle::TypeName[] = "Mayo::PropertyAngle";
template<> const char PropertyVelocity::TypeName[] = "Mayo::PropertyVelocity";

} // namespace Mayo
