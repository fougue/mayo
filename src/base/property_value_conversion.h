/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "span.h"

#include <string>
#include <variant>
#include <vector>

namespace Mayo {

// Alias of the std::variant<...> type used by custom PropertyValueConversion::Variant
using PropertyValueConversion_BaseVariantType = std::variant<
    std::monostate, bool, int, double, std::string, std::vector<uint8_t>
>;

// Variant type to be used when (de)serializing values
class PropertyValueConversion_Variant : public PropertyValueConversion_BaseVariantType {
public:
    PropertyValueConversion_Variant() = default;
    PropertyValueConversion_Variant(bool v);
    PropertyValueConversion_Variant(int v);
    PropertyValueConversion_Variant(float v);
    PropertyValueConversion_Variant(double v);
    PropertyValueConversion_Variant(const char* str);
    PropertyValueConversion_Variant(const std::string& str);
    PropertyValueConversion_Variant(Span<const uint8_t> bytes);

    bool isValid() const;
    bool toBool(bool* ok = nullptr) const;
    int toInt(bool* ok = nullptr) const;
    double toDouble(bool* ok = nullptr) const;

    std::string toString(bool* ok = nullptr) const;
    const std::string& toConstRefString(bool* ok = nullptr) const;

    std::vector<uint8_t> toByteArray(bool* ok = nullptr) const;
    Span<const uint8_t> toConstRefByteArray(bool* ok = nullptr) const;

    bool isConvertibleToConstRefString() const;
    bool isByteArray() const;
};

// Mechanism to convert value of a Property object to/from a basic variant type
class PropertyValueConversion {
public:
    using BaseVariantType = PropertyValueConversion_BaseVariantType;
    using Variant = PropertyValueConversion_Variant;

    int doubleToStringPrecision() const { return m_doubleToStringPrecision; }
    void setDoubleToStringPrecision(int prec) { m_doubleToStringPrecision = prec; }

    // Returns value of property `prop` as a value converted to Variant type
    virtual Variant toVariant(const Property& prop) const;

    // Changes value of property `prop` with value from `variant`
    // Returns true/false in case of success/failure
    // TODO Use maybe std::error_code instead of bool
    virtual bool fromVariant(Property* prop, const Variant& variant) const;

    // Utility function: copy values of source properties of `srcPropGroup` into destination
    // properties of `destPropGroup`
    // Each value is copied using the provided `conv` conversion object. Copy of a property value
    // happens only if the source/destination properties have the same key
    // The source/destination properties don't need to be in the same order and have the same size
    // Returns true/false in case of success/error
    static bool copyValues(
        PropertyGroup* destPropGroup,
        const PropertyGroup& srcPropGroup,
        const PropertyValueConversion& conv
    );

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
