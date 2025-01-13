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

// Mechanism to convert value of a Property object to/from a basic variant type
class PropertyValueConversion {
public:
    // Alias of the std::variant<...> type used by custom PropertyValueConversion::Variant
    using BaseVariantType = std::variant<
        std::monostate, bool, int, double, std::string, std::vector<uint8_t>
    >;
    // Variant type to be used when (de)serializing values
    class Variant : public BaseVariantType {
    public:
        Variant() = default;
        Variant(bool v);
        Variant(int v);
        Variant(float v);
        Variant(double v);
        Variant(const char* str);
        Variant(const std::string& str);
        Variant(Span<const uint8_t> bytes);

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
