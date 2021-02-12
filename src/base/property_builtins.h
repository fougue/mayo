/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "result.h"
#include "qmeta_gp_pnt.h"
#include "qmeta_gp_trsf.h"
#include "qmeta_quantity_color.h"
#include "qmeta_quantity.h"

#include <QtCore/QDateTime>
#include <QtCore/QVariant>
#include <type_traits>

namespace Mayo {

template<typename T>
class GenericProperty : public Property {
public:
    using ValueType = T;

    GenericProperty(PropertyGroup* grp, const TextId& name);

    const T& value() const;
    Result<void> setValue(const T& val);
    operator const T&() const { return this->value(); }

    QVariant valueAsVariant() const override;
    Result<void> setValueFromVariant(const QVariant& variant) override;

    const char* dynTypeName() const override;
    static const char TypeName[];

protected:
    T m_value = {};
};

template<typename T>
class PropertyScalarConstraints {
    static_assert(std::is_scalar<T>::value, "Requires scalar type");
public:
    using ValueType = T;

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

template<typename T>
class GenericScalarProperty :
        public GenericProperty<T>,
        public PropertyScalarConstraints<T>
{
public:
    using ValueType = T;
    GenericScalarProperty(PropertyGroup* grp, const TextId& name);
    GenericScalarProperty(
            PropertyGroup* grp, const TextId& name,
            T minimum, T maximum, T singleStep);
};

class BasePropertyQuantity :
        public Property,
        public PropertyScalarConstraints<double>
{
public:
    virtual Unit quantityUnit() const = 0;
    virtual double quantityValue() const = 0;
    virtual Result<void> setQuantityValue(double v) = 0;

    const char* dynTypeName() const override;
    static const char TypeName[];

protected:
    BasePropertyQuantity(PropertyGroup* grp, const TextId& name);
};

template<Unit UNIT>
class GenericPropertyQuantity : public BasePropertyQuantity {
public:
    using QuantityType = Quantity<UNIT>;

    GenericPropertyQuantity(PropertyGroup* grp, const TextId& name);

    Unit quantityUnit() const override;
    double quantityValue() const override;
    Result<void> setQuantityValue(double v) override;

    QVariant valueAsVariant() const override;
    Result<void> setValueFromVariant(const QVariant& variant) override;

    QuantityType quantity() const;
    Result<void> setQuantity(QuantityType qty);

private:
    QuantityType m_quantity = {};
};

using PropertyBool = GenericProperty<bool>;
using PropertyInt = GenericScalarProperty<int>;
using PropertyDouble = GenericScalarProperty<double>;
using PropertyCheckState = GenericProperty<Qt::CheckState>;
using PropertyQByteArray = GenericProperty<QByteArray>;
using PropertyQString = GenericProperty<QString>;
using PropertyQStringList = GenericProperty<QStringList>;
using PropertyQDateTime = GenericProperty<QDateTime>;
using PropertyOccColor = GenericProperty<Quantity_Color>;
using PropertyOccPnt = GenericProperty<gp_Pnt>;
using PropertyOccTrsf = GenericProperty<gp_Trsf>;

using PropertyLength = GenericPropertyQuantity<Unit::Length>;
using PropertyArea = GenericPropertyQuantity<Unit::Area>;
using PropertyVolume = GenericPropertyQuantity<Unit::Volume>;
using PropertyMass = GenericPropertyQuantity<Unit::Mass>;
using PropertyTime = GenericPropertyQuantity<Unit::Time>;
using PropertyAngle = GenericPropertyQuantity<Unit::Angle>;
using PropertyVelocity = GenericPropertyQuantity<Unit::Velocity>;

// --
// -- Implementation
// --

// GenericProperty<>

template<typename T>
GenericProperty<T>::GenericProperty(PropertyGroup* grp, const TextId& name)
    : Property(grp, name)
{ }

template<typename T> const T& GenericProperty<T>::value() const
{ return m_value; }

template<typename T> Result<void> GenericProperty<T>::setValue(const T& val)
{
    return Property::setValueHelper(this, &m_value, val);
}

template<typename T> QVariant GenericProperty<T>::valueAsVariant() const
{
    return QVariant::fromValue(this->value());
}

template<typename T>
Result<void> GenericProperty<T>::setValueFromVariant(const QVariant& variant)
{
    if (variant.canConvert<T>())
        return this->setValue(variant.value<T>());
    else
        return Result<void>::error("Incompatible type");
}

template<typename T> const char* GenericProperty<T>::dynTypeName() const
{ return TypeName; }

// PropertyScalarConstraints<>

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

// GenericScalarProperty<>

template<typename T>
GenericScalarProperty<T>::GenericScalarProperty(PropertyGroup* grp, const TextId& name)
    : GenericProperty<T>(grp, name)
{ }

template<typename T>
GenericScalarProperty<T>::GenericScalarProperty(
            PropertyGroup* grp, const TextId& name,
            T minimum, T maximum, T singleStep)
    : GenericProperty<T>(grp, name),
      PropertyScalarConstraints<T>(minimum, maximum, singleStep)
{ }

// GenericPropertyQuantity<>

template<Unit UNIT>
GenericPropertyQuantity<UNIT>::GenericPropertyQuantity(PropertyGroup* grp, const TextId& name)
    : BasePropertyQuantity(grp, name)
{ }

template<Unit UNIT>
Unit GenericPropertyQuantity<UNIT>::quantityUnit() const
{ return UNIT; }

template<Unit UNIT>
double GenericPropertyQuantity<UNIT>::quantityValue() const
{
    return this->quantity().value();
}

template<Unit UNIT>
Result<void> GenericPropertyQuantity<UNIT>::setQuantityValue(double v)
{
    return this->setQuantity(QuantityType(v));
}

template<Unit UNIT>
QVariant GenericPropertyQuantity<UNIT>::valueAsVariant() const
{
    return QVariant::fromValue(this->quantity());
}

template<Unit UNIT>
Result<void> GenericPropertyQuantity<UNIT>::setValueFromVariant(const QVariant& variant)
{
    if (variant.canConvert<QuantityType>())
        return this->setQuantity(variant.value<QuantityType>());
    else
        return Result<void>::error("Incompatible quantity type");
}

template<Unit UNIT>
Quantity<UNIT> GenericPropertyQuantity<UNIT>::quantity() const
{ return m_quantity; }

template<Unit UNIT>
Result<void> GenericPropertyQuantity<UNIT>::setQuantity(Quantity<UNIT> qty)
{
    return Property::setValueHelper(this, &m_quantity, qty);
}

} // namespace Mayo
