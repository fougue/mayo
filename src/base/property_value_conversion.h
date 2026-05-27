/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "property.h"

#include <gsl/span>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Mayo {

// Mechanism to convert value of a Property object to/from a basic variant type
//
// The conversion format is based on Variant, a lightweight std::variant-based type supporting a
// restricted set of serializable primitive values.
//
// Derived classes may specialize the conversion logic for specific Property subclasses or
// application domains
//
// String conversion of floating-point values uses the precision controlled by
// doubleToStringPrecision()
class PropertyValueConversion {
public:
    virtual ~PropertyValueConversion() = default;

    // Alias of the std::variant<...> type used by custom PropertyValueConversion::Variant
    using BaseVariantType = std::variant<
        std::monostate, bool, int, double, std::string, std::vector<uint8_t>
    >;
    // Variant type to be used when (de)serializing values
    class Variant : public BaseVariantType {
    public:
        using BaseVariantType::BaseVariantType;

        Variant(const char* str);
        Variant(gsl::span<const uint8_t> bytes);

        bool isValid() const;
        bool toBool(bool* ok = nullptr) const;
        int toInt(bool* ok = nullptr) const;
        double toDouble(bool* ok = nullptr) const;

        std::string toString(bool* ok = nullptr) const;
        const std::string& toConstRefString(bool* ok = nullptr) const;

        std::vector<uint8_t> toByteArray(bool* ok = nullptr) const;
        gsl::span<const uint8_t> toConstRefByteArray(bool* ok = nullptr) const;

        bool isConvertibleToConstRefString() const;
        bool isByteArray() const;
    };
    // Associative container mapping string keys to Variant values
    // Uses transparent string comparison(std::less<>) in order to support heterogeneous lookup with
    // std::string_view without requiring temporary std::string allocations
    using VariantMap = std::map<std::string, Variant, std::less<>>;

    int doubleToStringPrecision() const { return m_doubleToStringPrecision; }
    void setDoubleToStringPrecision(int prec) { m_doubleToStringPrecision = prec; }

    virtual Variant toVariant(const Property& prop) const;

    // TODO Use maybe std::error_code instead of bool
    virtual bool fromVariant(Property* prop, const Variant& variant) const;

protected:
    // Implementation helpers
    template<typename T> static const T& constRef(const Property& prop);
    template<typename T> static T* ptr(Property* prop);

    template<typename T> static bool isType(const Property& prop);
    template<typename T> static bool isType(const Property* prop);

private:
    int m_doubleToStringPrecision = 6;
};


// --
// -- Implementation
// --

template<typename T> const T& PropertyValueConversion::constRef(const Property& prop) {
    return static_cast<const T&>(prop);
}

template<typename T> T* PropertyValueConversion::ptr(Property* prop) {
    return static_cast<T*>(prop);
}

template<typename T> bool PropertyValueConversion::isType(const Property& prop) {
    return prop.dynTypeName() == T::TypeName;
}

template<typename T> bool PropertyValueConversion::isType(const Property* prop) {
    return prop ? isType<T>(*prop) : false;
}

} // namespace Mayo


namespace std {

template<> struct variant_size<Mayo::PropertyValueConversion::Variant>
    : variant_size<Mayo::PropertyValueConversion::BaseVariantType>
{};

template<size_t I> struct variant_alternative<I, Mayo::PropertyValueConversion::Variant>
    : variant_alternative<I, Mayo::PropertyValueConversion::BaseVariantType>
{};

} // namespace std
