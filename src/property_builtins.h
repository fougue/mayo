/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include "property.h"
#include <QtCore/QDateTime>
#include <Quantity_Color.hxx>
#include <type_traits>

namespace Mayo {

template<typename T> struct PropertyDefault
{ static T initValue() { return T(); } };

template<> struct PropertyDefault<bool>
{ static bool initValue() { return false; } };

template<> struct PropertyDefault<int>
{ static int initValue() { return 0; } };

template<> struct PropertyDefault<double>
{ static double initValue() { return 0.; } };

template<typename T, const char* TYPENAME_STR>
class GenericProperty : public Property
{
public:
    GenericProperty(PropertyOwner* owner, const QString& label);

    const T& value() const;
    void setValue(const T& val);

    const char* dynTypeName() const override;

protected:
    T m_value = PropertyDefault<T>::initValue();
};

template<typename T>
class PropertyScalarConstraints
{
    static_assert(std::is_scalar<T>::value, "Requires scalar type");

public:
    PropertyScalarConstraints() = default;
    PropertyScalarConstraints(T minimum, T maximum, T singleStep);

    bool constraintsEnabled() const;
    void setConstraintsEnabled(bool on);

    T minimum() const;
    void setMinimum(T val);

    T maximum() const;
    void setMaximum(T val);

    void setRange(T minVal, T maxVal);

    T singleStep() const;
    void setSingleStep(T step);

private:
    T m_minimum;
    T m_maximum;
    T m_singleStep;
    bool m_constraintsEnabled = false;
};

template<typename T, const char* TYPENAME_STR>
class GenericScalarProperty :
        public GenericProperty<T, TYPENAME_STR>,
        public PropertyScalarConstraints<T>
{
public:
    GenericScalarProperty(PropertyOwner* owner, const QString& label);
    GenericScalarProperty(
            PropertyOwner* owner, const QString& label,
            T minimum, T maximum, T singleStep);
};

typedef GenericProperty<bool, Property::BoolTypeName> PropertyBool;
typedef GenericScalarProperty<int, Property::IntTypeName> PropertyInt;
typedef GenericScalarProperty<double, Property::DoubleTypeName> PropertyDouble;
typedef GenericProperty<QByteArray, Property::QByteArrayTypeName> PropertyQByteArray;
typedef GenericProperty<QString, Property::QStringTypeName> PropertyQString;
typedef GenericProperty<QDateTime, Property::QDateTimeTypeName> PropertyQDateTime;
typedef GenericProperty<Quantity_Color, Property::OccColorTypeName> PropertyOccColor;



// --
// -- Implementation
// --

template<typename T, const char* TYPENAME_STR>
GenericProperty<T, TYPENAME_STR>::GenericProperty(
        PropertyOwner* owner, const QString& label)
    : Property(owner, label)
{ }

template<typename T, const char* TYPENAME_STR>
const T& GenericProperty<T, TYPENAME_STR>::value() const
{ return m_value; }

template<typename T, const char* TYPENAME_STR>
void GenericProperty<T, TYPENAME_STR>::setValue(const T& val)
{
    m_value = val;
    this->notifyChanged();
}

template<typename T, const char* TYPENAME_STR>
const char* GenericProperty<T, TYPENAME_STR>::dynTypeName() const
{ return TYPENAME_STR; }

template<typename T>
PropertyScalarConstraints<T>::PropertyScalarConstraints(
        T minimum, T maximum, T singleStep)
    : m_minimum(minimum),
      m_maximum(maximum),
      m_singleStep(singleStep),
      m_constraintsEnabled(true)
{ }

template<typename T> bool PropertyScalarConstraints<T>::constraintsEnabled() const
{ return m_constraintsEnabled; }

template<typename T> void PropertyScalarConstraints<T>::setConstraintsEnabled(bool on)
{ m_constraintsEnabled = on; }

template<typename T> T PropertyScalarConstraints<T>::minimum() const
{ return m_minimum; }

template<typename T> void PropertyScalarConstraints<T>::setMinimum(T val)
{ m_minimum = val; }

template<typename T> T PropertyScalarConstraints<T>::maximum() const
{ return m_maximum; }

template<typename T> void PropertyScalarConstraints<T>::setMaximum(T val)
{ m_maximum = val; }

template<typename T>
void PropertyScalarConstraints<T>::setRange(T minVal, T maxVal)
{
    this->setMinimum(minVal);
    this->setMaximum(maxVal);
}

template<typename T> T PropertyScalarConstraints<T>::singleStep() const
{ return m_singleStep; }

template<typename T> void PropertyScalarConstraints<T>::setSingleStep(T step)
{ m_singleStep = step; }

template<typename T, const char* TYPENAME_STR>
GenericScalarProperty<T, TYPENAME_STR>::GenericScalarProperty(
        PropertyOwner* owner, const QString& label)
    : GenericProperty(owner, label)
{ }

template<typename T, const char* TYPENAME_STR>
GenericScalarProperty<T, TYPENAME_STR>::GenericScalarProperty(
            PropertyOwner* owner, const QString& label,
            T minimum, T maximum, T singleStep)
    : GenericProperty(owner, label),
      PropertyScalarConstraints(minimum, maximum, singleStep)
{ }

} // namespace Mayo
