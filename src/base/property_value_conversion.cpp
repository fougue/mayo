/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "property_value_conversion.h"

#include "cpp_utils.h"
#include "filepath_conv.h"
#include "libfromchars.h"
#include "math_utils.h"
#include "property_builtins.h"
#include "property_enumeration.h"
#include "string_conv.h"
#include "tkernel_utils.h"
#include "unit_system.h"

#include <fmt/format.h>
#include <algorithm>
#include <charconv>
#include <iostream>
#include <stdexcept>

namespace Mayo {

namespace {

// Helper that converts a double number into a string
std::string toString(double value, int prec = 6)
{
#if __cpp_lib_to_chars
    char buff[64] = {};
    auto toCharsFormat = std::chars_format::general;
    auto resToChars = std::to_chars(std::begin(buff), std::end(buff), value, toCharsFormat, prec);
    if (resToChars.ec != std::errc())
        throw std::runtime_error("value_too_large");

    return std::string(buff, resToChars.ptr - buff);
#else
    return fmt::format("{:.{}g}", value, prec);
#endif
}

std::string toString(const gp_XYZ& coords, int prec = 6)
{
    const std::string strX = toString(coords.X(), prec);
    const std::string strY = toString(coords.Y(), prec);
    const std::string strZ = toString(coords.Z(), prec);
    return strX + ", " + strY + ", " + strZ;
}

gp_XYZ xyzFromString(std::string_view str)
{
    const char* ptrCoord = str.data();
    const char* const ptrCoordEnd = str.data() + str.size();
    auto fnParseNextCoord = [&]{
        const std::locale& locale = std::locale::classic();
        ptrCoord = std::find_if(ptrCoord, ptrCoordEnd, [&](char ch) { return !std::isspace(ch, locale); });
        auto ptrComma = std::find(ptrCoord, ptrCoordEnd, ',');
        double coord = 0;
        auto [ptr, err] = Mayo::fromChars(ptrCoord, ptrComma, coord);
        if (err != std::errc())
            throw std::runtime_error(std::make_error_code(err).message());

        ptrCoord = ptrComma + 1;
        return coord;
    };

    gp_XYZ coords;
    coords.SetX(fnParseNextCoord());
    coords.SetY(fnParseNextCoord());
    coords.SetZ(fnParseNextCoord());
    return coords;
}

void printFromVariantError(const Property* prop, std::string_view text)
{
    // TODO Use other output stream(dedicated Messenger object?)
    std::cerr << fmt::format("PropertyValueConversion::fromVariant() {} [ propertyName={}, propertyType={} ]",
                             text, prop->name().key, prop->dynTypeName())
              << std::endl;
}

// Helper function to set Property value from Variant
//   `prop`
//       the target property to set the value of
//   `variant`
//       the source variant value to convert from
//   `fnConvert`
//       the function used to get the value out of `variant`. This function must be one of
//       Variant::toBool(), toInt(), ...
template<typename PropertyType, typename ValueType>
bool propertyFromVariant(
        PropertyType* prop,
        const PropertyValueConversion::Variant& variant,
        ValueType (PropertyValueConversion::Variant::*fnConvertTo)(bool*) const
    )
{
    static_assert(std::is_base_of_v<Property, PropertyType>);

    bool ok = false;
    auto value = (variant.*fnConvertTo)(&ok);
    if (ok)
        return prop->setValue(value);
    else
        printFromVariantError(prop, fmt::format("Variant not convertible to {}", prop->dynTypeName()));

    return false;
}

// Helper function to set Property value from Variant holding a string representation of X,Y,Z coordinates
// See xyzFromString() function
template<typename PropertyType>
bool propertyFromVariantXyz(PropertyType* prop, const PropertyValueConversion::Variant& variant)
{
    static_assert(
        std::is_same_v<PropertyType, PropertyOccPnt> || std::is_same_v<PropertyType, PropertyOccVec>
    );

    if (variant.isConvertibleToConstRefString()) {
        try {
            return prop->setValue(xyzFromString(variant.toConstRefString()));
        } catch (const std::exception& err) {
            printFromVariantError(prop, fmt::format("Failed with error '{}'", err.what()));
        }
    }
    else {
        printFromVariantError(prop, "Variant expected to hold string");
    }

    return false;
}

// Helper function to set Property value from Variant holding a filepath specification
bool propertyFromVariantFilePath(PropertyFilePath* prop, const PropertyValueConversion::Variant& variant)
{
    // Note: explicit conversion from utf8 std::string to std::filesystem::path
    //       If character type of the source string is "char" then FilePath constructor assumes
    //       native narrow encoding(which might cause encoding issues)
    if (variant.isConvertibleToConstRefString())
        return prop->setValue(filepathFrom(variant.toConstRefString()));

    printFromVariantError(prop, "Variant expected to hold string");
    return false;
}

// Helper function to set Property value from Variant holding a string representation of hexadecimal
// color(format: #RRGGBB)
bool propertyFromVariantColor(PropertyOccColor* prop, const PropertyValueConversion::Variant& variant)
{
    if (variant.isConvertibleToConstRefString()) {
        const std::string& strColorHex = variant.toConstRefString();
        Quantity_Color color;
        if (TKernelUtils::colorFromHex(strColorHex, &color))
            return prop->setValue(color);
        else
            printFromVariantError(prop, fmt::format("Not hexadecimal format '{}'", strColorHex));
    }
    else {
        printFromVariantError(prop, "Variant expected to hold string");
    }

    return false;
}

// Helper function to set Property value from Variant holding the name of an enumerated value
bool propertyFromVariantEnumValue(PropertyEnumeration* prop, const PropertyValueConversion::Variant& variant)
{
    if (variant.isConvertibleToConstRefString()) {
        const std::string& name = variant.toConstRefString();
        const Enumeration::Item* ptrItem = prop->enumeration().findItemByName(name);
        if (ptrItem)
            return prop->setValue(ptrItem->value);

        printFromVariantError(prop, fmt::format("Found no enumeration item for '{}'", name));
    }
    else {
        printFromVariantError(prop, "Variant expected to hold string");
    }

    return false;
}

// Helper function to set Property value from Variant holding a string representation of a quantity
bool propertyFromVariantQuantity(BasePropertyQuantity* prop, const PropertyValueConversion::Variant& variant)
{
    if (variant.isConvertibleToConstRefString()) {
        const std::string& strQty = variant.toConstRefString();
        Unit unit;
        const UnitSystem::TranslateResult trRes = UnitSystem::parseQuantity(strQty, &unit);
        if (!trRes.strUnit || MathUtils::fuzzyIsNull(trRes.factor)) {
            printFromVariantError(prop, fmt::format("Failed to parse quantity string '{}'", strQty));
            return false;
        }

        if (unit != Unit::None && unit != prop->quantityUnit()) {
            printFromVariantError(prop, fmt::format("Unit mismatch with quantity string '{}'", strQty));
            return false;
        }

        return prop->setQuantityValue(trRes.value * trRes.factor);
    }

    printFromVariantError(prop, "Variant expected to hold string");
    return false;
}

} // namespace

PropertyValueConversion::Variant::Variant(const char* str)
    : BaseVariantType(std::string(str))
{}

PropertyValueConversion::Variant::Variant(gsl::span<const uint8_t> bytes)
    : BaseVariantType(std::vector<uint8_t>(bytes.begin(), bytes.end()))
{}

PropertyValueConversion::Variant PropertyValueConversion::toVariant(const Property& prop) const
{
    auto fnPrintError = [&](std::string_view text) {
        // TODO Use other output stream(dedicated Messenger object?)
        std::cerr << fmt::format("PropertyValueConversion::toVariant() {} [ propertyName={}, propertyType={} ]",
                                 text, prop.name().key, prop.dynTypeName())
                  << std::endl;
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
    else if (isType<PropertyString>(prop)) {
        return constRef<PropertyString>(prop).value();
    }
    else if (isType<PropertyFilePath>(prop)) {
        return constRef<PropertyFilePath>(prop).value().u8string();
    }
    else if (isType<PropertyOccPnt>(prop)) {
        try {
            return toString(constRef<PropertyOccPnt>(prop).value().XYZ(), m_doubleToStringPrecision);
        } catch (const std::exception& err) {
            fnPrintError(fmt::format("Failed with error: '{}'", err.what()));
        }
    }
    else if (isType<PropertyOccVec>(prop)) {
        try {
            return toString(constRef<PropertyOccVec>(prop).value().XYZ(), m_doubleToStringPrecision);
        } catch (const std::exception& err) {
            fnPrintError(fmt::format("Failed with error: '{}'", err.what()));
        }
    }
    else if (isType<PropertyOccColor>(prop)) {
        return TKernelUtils::colorToHex(constRef<PropertyOccColor>(prop));
    }
    else if (isType<PropertyEnumeration>(prop)) {
        return std::string(constRef<PropertyEnumeration>(prop).valueName());
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        const auto& qtyProp = constRef<BasePropertyQuantity>(prop);
        const auto trRes = UnitSystem::translate(UnitSystem::SI, qtyProp.quantityValue(), qtyProp.quantityUnit());
        try {
            const double value = trRes.value * trRes.factor;
            return toString(value, m_doubleToStringPrecision) + trRes.strUnit;
        } catch (const std::exception& err) {
            fnPrintError(fmt::format("Failed with error: '{}'", err.what()));
        }
    }
    else {
        fnPrintError("Support of this property type not yet implemented");
    }

    return {};
}

bool PropertyValueConversion::fromVariant(Property* prop, const Variant& variant) const
{
    if (!prop)
        return false;

    if (isType<PropertyBool>(prop)) {
        return propertyFromVariant(ptr<PropertyBool>(prop), variant, &Variant::toBool);
    }
    else if (isType<PropertyInt>(prop)) {
        return propertyFromVariant(ptr<PropertyInt>(prop), variant, &Variant::toInt);
    }
    else if (isType<PropertyDouble>(prop)) {
        return propertyFromVariant(ptr<PropertyDouble>(prop), variant, &Variant::toDouble);
    }
    else if (isType<PropertyString>(prop)) {
        if (variant.isConvertibleToConstRefString())
            return propertyFromVariant(ptr<PropertyString>(prop), variant, &Variant::toConstRefString);
        else
            return propertyFromVariant(ptr<PropertyString>(prop), variant, &Variant::toString);
    }
    else if (isType<PropertyFilePath>(prop)) {
        return propertyFromVariantFilePath(ptr<PropertyFilePath>(prop), variant);
    }
    else if (isType<PropertyOccPnt>(prop)) {
        return propertyFromVariantXyz(ptr<PropertyOccPnt>(prop), variant);
    }
    else if (isType<PropertyOccVec>(prop)) {
        return propertyFromVariantXyz(ptr<PropertyOccVec>(prop), variant);
    }
    else if (isType<PropertyOccColor>(prop)) {
        return propertyFromVariantColor(ptr<PropertyOccColor>(prop), variant);
    }
    else if (isType<PropertyEnumeration>(prop)) {
        return propertyFromVariantEnumValue(ptr<PropertyEnumeration>(prop), variant);
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        return propertyFromVariantQuantity(ptr<BasePropertyQuantity>(prop), variant);
    }
    else {
        printFromVariantError(prop, fmt::format("Support of {} not yet implemented", prop->dynTypeName()));
    }

    return false;
}

static void assignBoolPtr(bool* value, bool on)
{
    if (value)
        *value = on;
}

bool PropertyValueConversion::Variant::isValid() const
{
    return this->index() != 0; // not std::monostate
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
    if (std::holds_alternative<int>(*this)) {
        return std::get<int>(*this);
    }
    else if (std::holds_alternative<double>(*this)) {
        auto dval = std::floor(std::get<double>(*this));
        if (std::isgreaterequal(dval, INT_MIN) && std::islessequal(dval, INT_MAX))
            return static_cast<int>(dval);
    }
    else if (std::holds_alternative<std::string>(*this)) {
        try {
            return std::stoi(std::get<std::string>(*this));
        }
        catch (const std::exception&) {
        }
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(*this)) {
        try {
            return std::stoi(this->toString(ok));
        }
        catch (const std::exception&) {
        }
    }

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
    if (std::holds_alternative<int>(*this)) {
        return std::to_string(std::get<int>(*this));
    }
    else if (std::holds_alternative<double>(*this)) {
        return std::to_string(std::get<double>(*this));
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(*this)) {
        const auto bytes = this->toConstRefByteArray(ok);
        return std::string{ bytes.begin(), bytes.end() };
    }
    else {
        return this->toConstRefString();
    }
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
    return Cpp::staticObject<std::string>();
}

std::vector<uint8_t> PropertyValueConversion::Variant::toByteArray(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<std::string>(*this)) {
        const std::string& str = std::get<std::string>(*this);
        std::vector<uint8_t> bytes;
        bytes.resize(str.size());
        std::copy(str.cbegin(), str.cend(), bytes.begin());
        return bytes;
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(*this)) {
        return std::get<std::vector<uint8_t>>(*this);
    }

    assignBoolPtr(ok, false);
    return {};
}

gsl::span<const uint8_t> PropertyValueConversion::Variant::toConstRefByteArray(bool* ok) const
{
    assignBoolPtr(ok, true);
    if (std::holds_alternative<std::vector<uint8_t>>(*this))
        return std::get<std::vector<uint8_t>>(*this);

    assignBoolPtr(ok, false);
    return {};
}

bool PropertyValueConversion::Variant::isConvertibleToConstRefString() const
{
    return std::holds_alternative<bool>(*this) || std::holds_alternative<std::string>(*this);
}

bool PropertyValueConversion::Variant::isByteArray() const
{
    return std::holds_alternative<std::vector<uint8_t>>(*this);
}

} // namespace Mayo
