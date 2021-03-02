/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_builtins.h"

#include "string_utils.h"

namespace Mayo {

BasePropertyQuantity::BasePropertyQuantity(PropertyGroup* grp, const TextId& name)
    : Property(grp, name)
{
}

template<> const char PropertyBool::TypeName[] = "Mayo::PropertyBool";
template<> const char GenericProperty<int>::TypeName[] = "Mayo::PropertyInt";
template<> const char GenericProperty<double>::TypeName[] = "Mayo::PropertyDouble";
template<> const char GenericProperty<Qt::CheckState>::TypeName[] = "Mayo::PropertyCheckState";
template<> const char PropertyQByteArray::TypeName[] = "Mayo::PropertyQByteArray";
template<> const char PropertyQString::TypeName[] = "Mayo::PropertyQString";
template<> const char PropertyQStringList::TypeName[] = "Mayo::PropertyQStringList";
template<> const char PropertyQDateTime::TypeName[] = "Mayo::PropertyQDateTime";
template<> const char PropertyOccColor::TypeName[] = "Mayo::PropertyOccColor";
template<> const char PropertyOccPnt::TypeName[] = "Mayo::PropertyOccPnt";
template<> const char PropertyOccTrsf::TypeName[] = "Mayo::PropertyOccTrsf";

PropertyOccColor::PropertyOccColor(PropertyGroup* grp, const TextId& name)
    : GenericProperty<Quantity_Color>(grp, name)
{
}

QVariant PropertyOccColor::valueAsVariant() const
{
    constexpr bool hashPrefix = true;
    return StringUtils::fromUtf8(Quantity_Color::ColorToHex(this->value(), hashPrefix));
}

Result<void> PropertyOccColor::setValueFromVariant(const QVariant& variant)
{
    auto strColorHex = StringUtils::toUtf8<std::string>(variant.toString());
    Quantity_Color color;
    const bool ok = Quantity_Color::ColorFromHex(strColorHex.c_str(), color);
    if (ok)
        return this->setValue(color);

    return Result<void>::error(QString("Not hexadecimal format '%1'").arg(variant.toString()));
}

} // namespace Mayo
