/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"

#include <QtCore/QVariant>

namespace Mayo {

// Mechanism to convert value of a Property object to/from QVariant
class PropertyValueConversion {
public:
    //using Variant = std::variant<bool, int, double, std::string>;

    int doubleToStringPrecision() const { return m_doubleToStringPrecision; }
    void setDoubleToStringPrecision(int prec) { m_doubleToStringPrecision = prec; }

    virtual QVariant toVariant(const Property& prop) const;

    // TODO Use maybe std::error_code instead of bool
    virtual bool fromVariant(Property* prop, const QVariant& variant) const;

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
