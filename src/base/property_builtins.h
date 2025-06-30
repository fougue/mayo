/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "global.h"
#include "property.h"
#include "filepath.h"
#include "quantity.h"

#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Quantity_Color.hxx>

#include <string>
#include <type_traits>

namespace Mayo {

template<typename T>
class GenericProperty : public Property {
public:
    using ValueType = T;
    // TODO Add value_type traits for T

    GenericProperty(PropertyGroup* grp, const TextId& name);

    const T& value() const { return m_value; }
    bool setValue(const T& val);
    operator const T&() const { return this->value(); }

    const char* dynTypeName() const override { return TypeName; }
    static const char TypeName[];

protected:
    T m_value = {};
};

template<typename T>
class PropertyScalarConstraints {
    static_assert(std::is_scalar_v<T>, "Requires scalar type");
public:
    using ValueType = T;

    PropertyScalarConstraints() = default;
    PropertyScalarConstraints(T minimum, T maximum, T singleStep);

    bool constraintsEnabled() const { return m_constraintsEnabled; }
    void setConstraintsEnabled(bool on) { m_constraintsEnabled = on; }

    T minimum() const { return m_minimum; }
    void setMinimum(T val) { m_minimum = val; }

    T maximum() const { return m_maximum; }
    void setMaximum(T val) { m_maximum = val; }

    void setRange(T minVal, T maxVal);

    T singleStep() const { return m_singleStep; }
    void setSingleStep(T step) { m_singleStep = step; }

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
        T minimum, T maximum, T singleStep
    );
};

class BasePropertyQuantity :
        public Property,
        public PropertyScalarConstraints<double>
{
public:
    virtual Unit quantityUnit() const = 0;
    virtual double quantityValue() const = 0;
    virtual bool setQuantityValue(double v) = 0;

    inline static const char TypeName[] = "Mayo::BasePropertyQuantity";
    const char* dynTypeName() const override { return BasePropertyQuantity::TypeName; }

protected:
    BasePropertyQuantity(PropertyGroup* grp, const TextId& name);
};

template<Unit U>
class GenericPropertyQuantity : public BasePropertyQuantity {
public:
    using QuantityType = Quantity<U>;

    GenericPropertyQuantity(PropertyGroup* grp, const TextId& name);

    Unit quantityUnit() const override { return U; }
    double quantityValue() const override { return this->quantity().value(); }
    bool setQuantityValue(double v) override;

    QuantityType quantity() const { return m_quantity; }
    bool setQuantity(QuantityType qty);

private:
    QuantityType m_quantity = {};
};

using PropertyBool = GenericProperty<bool>;
using PropertyInt = GenericScalarProperty<int>;
using PropertyDouble = GenericScalarProperty<double>;
using PropertyString = GenericProperty<std::string>;
using PropertyCheckState = GenericProperty<CheckState>;
using PropertyOccPnt = GenericProperty<gp_Pnt>;
using PropertyOccVec = GenericProperty<gp_Vec>;
using PropertyOccTrsf = GenericProperty<gp_Trsf>;
using PropertyOccColor = GenericProperty<Quantity_Color>;
using PropertyFilePath = GenericProperty<FilePath>;

using PropertyLength = GenericPropertyQuantity<Unit::Length>;
using PropertyArea = GenericPropertyQuantity<Unit::Area>;
using PropertyVolume = GenericPropertyQuantity<Unit::Volume>;
using PropertyMass = GenericPropertyQuantity<Unit::Mass>;
using PropertyTime = GenericPropertyQuantity<Unit::Time>;
using PropertyAngle = GenericPropertyQuantity<Unit::Angle>;
using PropertyVelocity = GenericPropertyQuantity<Unit::Velocity>;
using PropertyDensity = GenericPropertyQuantity<Unit::Density>;

// --
// -- Implementation
// --

// GenericProperty<>

template<typename T>
GenericProperty<T>::GenericProperty(PropertyGroup* grp, const TextId& name)
    : Property(grp, name)
{ }

template<typename T> bool GenericProperty<T>::setValue(const T& val)
{
    return Property::setValueHelper(this, &m_value, val);
}

// PropertyScalarConstraints<>

template<typename T>
PropertyScalarConstraints<T>::PropertyScalarConstraints(T minimum, T maximum, T singleStep)
    : m_minimum(minimum),
      m_maximum(maximum),
      m_singleStep(singleStep),
      m_constraintsEnabled(true)
{ }

template<typename T>
void PropertyScalarConstraints<T>::setRange(T minVal, T maxVal)
{
    this->setMinimum(minVal);
    this->setMaximum(maxVal);
}

// GenericScalarProperty<>

template<typename T>
GenericScalarProperty<T>::GenericScalarProperty(PropertyGroup* grp, const TextId& name)
    : GenericProperty<T>(grp, name)
{ }

template<typename T>
GenericScalarProperty<T>::GenericScalarProperty(
        PropertyGroup* grp, const TextId& name,
        T minimum, T maximum, T singleStep
    )
    : GenericProperty<T>(grp, name),
      PropertyScalarConstraints<T>(minimum, maximum, singleStep)
{ }

// GenericPropertyQuantity<>

template<Unit U>
GenericPropertyQuantity<U>::GenericPropertyQuantity(PropertyGroup* grp, const TextId& name)
    : BasePropertyQuantity(grp, name)
{ }

template<Unit U>
bool GenericPropertyQuantity<U>::setQuantityValue(double v)
{
    return this->setQuantity(QuantityType(v));
}

template<Unit U>
bool GenericPropertyQuantity<U>::setQuantity(Quantity<U> qty)
{
    return Property::setValueHelper(this, &m_quantity, qty);
}

} // namespace Mayo
