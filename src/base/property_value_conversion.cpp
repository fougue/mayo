/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_value_conversion.h"

#include "cpp_utils.h"
#include "property_builtins.h"
#include "property_enumeration.h"
#include "string_conv.h"
#include "tkernel_utils.h"
#include "unit_system.h"

#include <QtCore/QtDebug>
#include <fmt/format.h>
#include <charconv>
#include <sstream>

namespace Mayo {

PropertyValueConversion::Variant PropertyValueConversion::toVariant(const Property& prop) const
{
    auto fnError = [=](std::string_view text) {
        qCritical() << to_QString(text);
        return PropertyValueConversion::Variant{};
    };

    if (isType<PropertyBool>(prop)) {
        return constRef<PropertyBool>(prop).value();
    }
    else if (isType<PropertyInt>(prop)) {
        return constRef<PropertyInt>(prop).value();
    }
    else if (isType<PropertyDouble>(prop)) {
        return constRef<PropertyDouble>(prop).value();
    }
    else if (isType<PropertyCheckState>(prop)) {
        return fnError("toVariant() not yet implemented for PropertyCheckState");
    }
    else if (isType<PropertyString>(prop)) {
        return constRef<PropertyString>(prop).value();
    }
    else if (isType<PropertyFilePath>(prop)) {
        return constRef<PropertyFilePath>(prop).value().u8string();
    }
    else if (isType<PropertyOccPnt>(prop)) {
        return fnError("toVariant() not yet implemented for PropertyOccPnt");
    }
    else if (isType<PropertyOccTrsf>(prop)) {
        return fnError("toVariant() not yet implemented for PropertyOccTrsf");
    }
    else if (isType<PropertyOccColor>(prop)) {
        return TKernelUtils::colorToHex(constRef<PropertyOccColor>(prop));
    }
    else if (isType<PropertyEnumeration>(prop)) {
        return std::string(constRef<PropertyEnumeration>(prop).name());
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        const auto& qtyProp = constRef<BasePropertyQuantity>(prop);
        const UnitSystem::TranslateResult trRes = UnitSystem::translate(
                    UnitSystem::SI, qtyProp.quantityValue(), qtyProp.quantityUnit());
#if __cpp_lib_to_chars
        char buff[64] = {};
        auto resToChars = std::to_chars(
                    std::begin(buff),
                    std::end(buff),
                    trRes.value * trRes.factor,
                    std::chars_format::general,
                    m_doubleToStringPrecision);
        if (resToChars.ec == std::errc())
            return std::string(buff, resToChars.ptr - buff) + trRes.strUnit;
        else
            return fnError("toVariant(PropertyQuantity) failed with error: 'value_too_large'");
#else
        std::stringstream sstr;
        sstr.precision(m_doubleToStringPrecision);
        sstr << trRes.value * trRes.factor;
        return sstr.str() + trRes.strUnit;
#endif
    }

    return {};
}

bool PropertyValueConversion::fromVariant(Property* prop, const Variant& variant) const
{
    if (!prop)
        return false;

    auto fnError = [=](std::string_view text) {
        qCritical() << QString("[property '%1'] %2").arg(to_QString(prop->name().key), to_QString(text));
        return false;
    };

    if (isType<PropertyBool>(prop)) {
        bool okConversion = false;
        const bool on = variant.toBool(&okConversion);
        if (okConversion)
            return ptr<PropertyBool>(prop)->setValue(on);

        return fnError("fromVariant(PropertyBool) variant not convertible to bool");
    }
    else if (isType<PropertyInt>(prop)) {
        bool okConversion = false;
        const int v = variant.toInt(&okConversion);
        if (okConversion)
            return ptr<PropertyInt>(prop)->setValue(v);

        return fnError("fromVariant(PropertyInt) variant not convertible to int");
    }
    else if (isType<PropertyDouble>(prop)) {
        bool okConversion = false;
        const double v = variant.toDouble(&okConversion);
        if (okConversion)
            return ptr<PropertyDouble>(prop)->setValue(v);

        return fnError("fromVariant(PropertyDouble) variant not convertible to double");
    }
    else if (isType<PropertyCheckState>(prop)) {
        return fnError("fromVariant() not yet implemented for PropertyCheckState");
    }
    else if (isType<PropertyString>(prop)) {
        if (variant.isConvertibleToConstRefString())
            return ptr<PropertyString>(prop)->setValue(variant.toConstRefString());

        bool okConversion = false;
        const std::string str = variant.toString(&okConversion);
        if (okConversion)
            return ptr<PropertyString>(prop)->setValue(str);

        return fnError("fromVariant(PropertyString) variant not convertible to string");
    }
    else if (isType<PropertyFilePath>(prop)) {
        if (variant.isConvertibleToConstRefString())
            return ptr<PropertyFilePath>(prop)->setValue(variant.toConstRefString());
        else
            return fnError("fromVariant(PropertyFilePath) variant expected to hold string");
    }
    else if (isType<PropertyOccPnt>(prop)) {
        return fnError("fromVariant() not yet implemented for PropertyOccPnt");
    }
    else if (isType<PropertyOccTrsf>(prop)) {
        return fnError("fromVariant() not yet implemented for PropertyOccTrsf");
    }
    else if (isType<PropertyOccColor>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            const std::string& strColorHex = variant.toConstRefString();
            Quantity_Color color;
            if (!TKernelUtils::colorFromHex(strColorHex, &color))
                return fnError(fmt::format("Not hexadecimal format '{}'", strColorHex));

            return ptr<PropertyOccColor>(prop)->setValue(color);
        }
        else {
            return fnError("fromVariant(PropertyOccColor) variant expected to hold string");
        }
    }
    else if (isType<PropertyEnumeration>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            const std::string& name = variant.toConstRefString();
            const Enumeration::Item* ptrItem = ptr<PropertyEnumeration>(prop)->enumeration().findItem(name);
            if (ptrItem)
                return ptr<PropertyEnumeration>(prop)->setValue(ptrItem->value);

            return fnError(fmt::format("fromVariant() found no enumeration item for '{}'", name));
        }
        else {
            return fnError("fromVariant(PropertyEnumeration) variant expected to hold string");
        }
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            const std::string& strQty = variant.toConstRefString();
            Unit unit;
            const UnitSystem::TranslateResult trRes = UnitSystem::parseQuantity(strQty, &unit);
            if (!trRes.strUnit || qFuzzyIsNull(trRes.factor))
                return fnError(fmt::format("fromVariant() failed to parse quantity string '{}'", strQty));

            if (unit != Unit::None && unit != ptr<BasePropertyQuantity>(prop)->quantityUnit())
                return fnError(fmt::format("fromVariant() unit mismatch with quantity string '{}'", strQty));

            return ptr<BasePropertyQuantity>(prop)->setQuantityValue(trRes.value * trRes.factor);
        }
        else {
            return fnError("fromVariant(BasePropertyQuantity) variant expected to hold string");
        }
    }

    return false;
}

static void assignBoolPtr(bool* value, bool on)
{
    if (value)
        *value = on;
}

bool PropertyValueConversion::Variant::toBool(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<bool>(*this))
        return std::get<bool>(*this);
    else if (std::holds_alternative<int>(*this))
        return std::get<int>(*this) != 0;

    assignBoolPtr(ok, false);
    return false;
}

int PropertyValueConversion::Variant::toInt(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<int>(*this))
        return std::get<int>(*this);
    else if (std::holds_alternative<double>(*this))
        return std::get<double>(*this);
    else if (std::holds_alternative<std::string>(*this))
        return std::stoi(std::get<std::string>(*this));

    assignBoolPtr(ok, false);
    return 0;
}

double PropertyValueConversion::Variant::toDouble(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<int>(*this))
        return std::get<int>(*this);
    else if (std::holds_alternative<double>(*this))
        return std::get<double>(*this);
    else if (std::holds_alternative<std::string>(*this))
        return std::stod(std::get<std::string>(*this));

    assignBoolPtr(ok, false);
    return 0.;
}

std::string PropertyValueConversion::Variant::toString(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<int>(*this))
        return std::to_string(std::get<int>(*this));
    else if (std::holds_alternative<double>(*this))
        return std::to_string(std::get<double>(*this));
    else
        return this->toConstRefString();
}

const std::string& PropertyValueConversion::Variant::toConstRefString(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<bool>(*this)) {
        static const std::string strTrue = "true";
        static const std::string strFalse = "false";
        return std::get<bool>(*this) ? strTrue : strFalse;
    }
    else if (std::holds_alternative<std::string>(*this)) {
        return std::get<std::string>(*this);
    }

    assignBoolPtr(ok, false);
    return CppUtils::nullString();
}

bool PropertyValueConversion::Variant::isConvertibleToConstRefString() const
{
    return std::holds_alternative<bool>(*this) || std::holds_alternative<std::string>(*this);
}

bool PropertyValueConversion::Variant::isByteArray() const
{
    return m_isByteArray && std::holds_alternative<std::string>(*this);
}

void PropertyValueConversion::Variant::setByteArray(bool on)
{
    m_isByteArray = on;
}

} // namespace Mayo
