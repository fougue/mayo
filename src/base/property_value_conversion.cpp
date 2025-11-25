/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_value_conversion.h"

#include "cpp_utils.h"
#include "filepath_conv.h"
#include "math_utils.h"
#include "property_builtins.h"
#include "property_enumeration.h"
#include "string_conv.h"
#include "tkernel_utils.h"
#include "unit_system.h"

#include <fmt/format.h>
#include <algorithm>
#if __cpp_lib_to_chars
#  include <charconv>
#else
#  include <cstdlib>
#  include <cstring>
#  include <sstream>
#endif
#include <iostream>
#include <stdexcept>

namespace Mayo {

namespace {

// Helper that converts a double number into a string
static std::string toString(double value, int prec = 6)
{
#if __cpp_lib_to_chars
    char buff[64] = {};
    auto toCharsFormat = std::chars_format::general;
    auto resToChars = std::to_chars(std::begin(buff), std::end(buff), value, toCharsFormat, prec);
    if (resToChars.ec != std::errc())
        throw std::runtime_error("value_too_large");

    return std::string(buff, resToChars.ptr - buff);
#else
    std::stringstream sstr;
    sstr.precision(prec);
    sstr << value;
    return sstr.str();
#endif
}

static std::string toString(const gp_XYZ& coords, int prec = 6)
{
    const std::string strX = toString(coords.X(), prec);
    const std::string strY = toString(coords.Y(), prec);
    const std::string strZ = toString(coords.Z(), prec);
    return strX + ", " + strY + ", " + strZ;
}

static gp_XYZ xyzFromString(std::string_view str)
{
    const char* ptrCoord = str.data();
    const char* const ptrCoordEnd = str.data() + str.size();
    auto fnParseNextCoord = [&]{
        const std::locale& locale = std::locale::classic();
        ptrCoord = std::find_if(ptrCoord, ptrCoordEnd, [&](char ch) { return !std::isspace(ch, locale); });
        auto ptrComma = std::find(ptrCoord, ptrCoordEnd, L',');
        double coord = 0;
#if __cpp_lib_to_chars
        auto [ptr, err] = std::from_chars(ptrCoord, ptrComma, coord);
        if (err != std::errc())
            throw std::runtime_error(std::make_error_code(err).message());
#else
        errno = 0;
        coord = std::strtod(ptrCoord, nullptr);
        if (errno != 0)
            throw std::runtime_error(std::strerror(errno));
#endif

        ptrCoord = ptrComma + 1;
        return coord;
    };

    gp_XYZ coords;
    coords.SetX(fnParseNextCoord());
    coords.SetY(fnParseNextCoord());
    coords.SetZ(fnParseNextCoord());
    return coords;
}

} // namespace

PropertyValueConversion::Variant::Variant(bool v)
    : BaseVariantType(v)
{}

PropertyValueConversion::Variant::Variant(int v)
    : BaseVariantType(v)
{}

PropertyValueConversion::Variant::Variant(float v)
    : Variant(static_cast<double>(v))
{}

PropertyValueConversion::Variant::Variant(double v)
    : BaseVariantType(v)
{}

PropertyValueConversion::Variant::Variant(const char* str)
    : BaseVariantType(std::string(str))
{}

PropertyValueConversion::Variant::Variant(const std::string& str)
    : BaseVariantType(str)
{}

PropertyValueConversion::Variant::Variant(Span<const uint8_t> bytes)
    : BaseVariantType(std::vector<uint8_t>(bytes.begin(), bytes.end()))
{}

PropertyValueConversion::Variant PropertyValueConversion::toVariant(const Property& prop) const
{
    auto fnError = [&](std::string_view text) {
        // TODO Use other output stream(dedicated Messenger object?)
        std::cerr << fmt::format("PropertyValueConversion::toVariant() {} [ propertyName={}, propertyType={} ]",
                                 text, prop.name().key, prop.dynTypeName())
                  << std::endl;
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
        return fnError("Support of this property type not yet implemented");
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
            return fnError(fmt::format("Failed with error: '{}'", err.what()));
        }
    }
    else if (isType<PropertyOccVec>(prop)) {
        try {
            return toString(constRef<PropertyOccVec>(prop).value().XYZ(), m_doubleToStringPrecision);
        } catch (const std::exception& err) {
            return fnError(fmt::format("Failed with error: '{}'", err.what()));
        }
    }
    else if (isType<PropertyOccTrsf>(prop)) {
        return fnError("Support of this property type not yet implemented");
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
            return fnError(fmt::format("Failed with error: '{}'", err.what()));
        }
    }

    return {};
}

bool PropertyValueConversion::fromVariant(Property* prop, const Variant& variant) const
{
    if (!prop)
        return false;

    auto fnError = [=](std::string_view text) {
        // TODO Use other output stream(dedicated Messenger object?)
        std::cerr << fmt::format("PropertyValueConversion::fromVariant() {} [ propertyName={}, propertyType={} ]",
                                 text, prop->name().key, prop->dynTypeName())
                  << std::endl;
        return false;
    };

    if (isType<PropertyBool>(prop)) {
        bool okConversion = false;
        const bool on = variant.toBool(&okConversion);
        if (okConversion)
            return ptr<PropertyBool>(prop)->setValue(on);

        return fnError("Variant not convertible to bool");
    }
    else if (isType<PropertyInt>(prop)) {
        bool okConversion = false;
        const int v = variant.toInt(&okConversion);
        if (okConversion)
            return ptr<PropertyInt>(prop)->setValue(v);

        return fnError("Variant not convertible to int");
    }
    else if (isType<PropertyDouble>(prop)) {
        bool okConversion = false;
        const double v = variant.toDouble(&okConversion);
        if (okConversion)
            return ptr<PropertyDouble>(prop)->setValue(v);

        return fnError("Variant not convertible to double");
    }
    else if (isType<PropertyCheckState>(prop)) {
        return fnError("Support of this property type not yet implemented");
    }
    else if (isType<PropertyString>(prop)) {
        if (variant.isConvertibleToConstRefString())
            return ptr<PropertyString>(prop)->setValue(variant.toConstRefString());

        bool okConversion = false;
        const std::string str = variant.toString(&okConversion);
        if (okConversion)
            return ptr<PropertyString>(prop)->setValue(str);

        return fnError("Variant not convertible to string");
    }
    else if (isType<PropertyFilePath>(prop)) {
        // Note: explicit conversion from utf8 std::string to std::filesystem::path
        //       If character type of the source string is "char" then FilePath constructor assumes
        //       native narrow encoding(which might cause encoding issues)
        if (variant.isConvertibleToConstRefString())
            return ptr<PropertyFilePath>(prop)->setValue(filepathFrom(variant.toConstRefString()));
        else
            return fnError("Variant expected to hold string");
    }
    else if (isType<PropertyOccPnt>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            try {
                return ptr<PropertyOccPnt>(prop)->setValue(xyzFromString(variant.toConstRefString()));
            } catch (const std::exception& err) {
                return fnError(fmt::format("Failed with error '{}'", err.what()));
            }
        }

        return fnError("Variant expected to hold string");
    }
    else if (isType<PropertyOccVec>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            try {
                return ptr<PropertyOccVec>(prop)->setValue(xyzFromString(variant.toConstRefString()));
            } catch (const std::exception& err) {
                return fnError(fmt::format("Failed with error '{}'", err.what()));
            }
        }

        return fnError("Variant expected to hold string");
    }
    else if (isType<PropertyOccTrsf>(prop)) {
        return fnError("Support of this property type not yet implemented");
    }
    else if (isType<PropertyOccColor>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            const std::string& strColorHex = variant.toConstRefString();
            Quantity_Color color;
            if (!TKernelUtils::colorFromHex(strColorHex, &color))
                return fnError(fmt::format("Not hexadecimal format '{}'", strColorHex));

            return ptr<PropertyOccColor>(prop)->setValue(color);
        }

        return fnError("Variant expected to hold string");
    }
    else if (isType<PropertyEnumeration>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            const std::string& name = variant.toConstRefString();
            const Enumeration::Item* ptrItem = ptr<PropertyEnumeration>(prop)->enumeration().findItemByName(name);
            if (ptrItem)
                return ptr<PropertyEnumeration>(prop)->setValue(ptrItem->value);

            return fnError(fmt::format("Found no enumeration item for '{}'", name));
        }

        return fnError("Variant expected to hold string");
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        if (variant.isConvertibleToConstRefString()) {
            const std::string& strQty = variant.toConstRefString();
            Unit unit;
            const UnitSystem::TranslateResult trRes = UnitSystem::parseQuantity(strQty, &unit);
            if (!trRes.strUnit || MathUtils::fuzzyIsNull(trRes.factor))
                return fnError(fmt::format("Failed to parse quantity string '{}'", strQty));

            if (unit != Unit::None && unit != ptr<BasePropertyQuantity>(prop)->quantityUnit())
                return fnError(fmt::format("Unit mismatch with quantity string '{}'", strQty));

            return ptr<BasePropertyQuantity>(prop)->setQuantityValue(trRes.value * trRes.factor);
        }

        return fnError("Variant expected to hold string");
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
    return CppUtils::nullString();
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

Span<const uint8_t> PropertyValueConversion::Variant::toConstRefByteArray(bool* ok) const
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
