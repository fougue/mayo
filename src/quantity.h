/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "unit.h"

namespace Mayo {

template<Unit UNIT> class Quantity {
    using Qty = Quantity<UNIT>;
public:
    Quantity() = default;
    explicit Quantity(double value): m_value(value) {}

    double value() const { return m_value; }
    void setValue(double v) { m_value = v; }
    Unit unit() const { return UNIT; }

    Qty operator/(double v) const { return Qty(m_value / v); }
    Qty operator+(const Qty& q) const { return Qty(m_value + q.value()); }
    Qty operator-(const Qty& q) const { return Qty(m_value - q.value()); }
    Qty operator-() const { return Qty(-m_value); }

    bool operator==(const Qty& q) const { return m_value == q.value(); }
    bool operator<(const Qty& q) const { return m_value < q.value(); }
    bool operator>(const Qty& q) const { return m_value > q.value(); }
    bool operator<=(const Qty& q) const { return m_value <= q.value(); }
    bool operator>=(const Qty& q) const { return m_value >= q.value(); }

    Qty& operator+=(const Qty& q) { return this->change(m_value + q.value()); }
    Qty& operator-=(const Qty& q) { return this->change(m_value - q.value()); }
    Qty& operator=(const Qty& q) { return this->change(q.value()); }

private:
    Qty& change(double v) {
        m_value = v;
        return *this;
    }

    double m_value;
};

using QuantityLength = Quantity<Unit::Length>;
using QuantityArea = Quantity<Unit::Area>;
using QuantityVolume = Quantity<Unit::Volume>;
using QuantityMass = Quantity<Unit::Mass>;
using QuantityTime = Quantity<Unit::Time>;
using QuantityAngle = Quantity<Unit::Angle>;
using QuantityVelocity = Quantity<Unit::Velocity>;

template<Unit UNIT>
Quantity<UNIT> operator*(const Quantity<UNIT>& lhs, double rhs) {
    return Quantity<UNIT>(lhs.value() * rhs);
}

template<Unit UNIT>
Quantity<UNIT> operator*(double lhs, const Quantity<UNIT>& rhs) {
    return rhs * lhs;
}

QuantityArea operator*(const QuantityLength& lhs, const QuantityLength& rhs);
QuantityVolume operator*(const QuantityLength& lhs, const QuantityArea& rhs);
QuantityVolume operator*(const QuantityArea& lhs, const QuantityLength& rhs);
QuantityVelocity operator/(const QuantityLength& lhs, const QuantityTime& rhs);
QuantityTime operator/(const QuantityLength& lhs, const QuantityVelocity& rhs);

const QuantityLength Quantity_Nanometer(1e-6);
const QuantityLength Quantity_Micrometer(1e-3);
const QuantityLength Quantity_Millimeter(1.);
const QuantityLength Quantity_Centimeter(10.);
const QuantityLength Quantity_Decimeter(100.);
const QuantityLength Quantity_Meter(1000.);
const QuantityLength Quantity_Kilometre(1e6);

const QuantityArea Quantity_SquaredMillimeter(1.);
const QuantityArea Quantity_SquaredCentimer(100.);
const QuantityArea Quantity_SquaredMeter(1e6);
const QuantityArea Quantity_SquaredKilometer(1e12);

const QuantityVolume Quantity_CubicMillimeter(1.);
const QuantityVolume Quantity_CubicCentimer(1000.);
const QuantityVolume Quantity_CubicMeter(1e9);

const QuantityLength Quantity_Inch(25.4);
const QuantityLength Quantity_Foot(304.8);
const QuantityLength Quantity_Thou(0.0254);
const QuantityLength Quantity_Yard(914.4);
const QuantityLength Quantity_Mile(1609344.);

const QuantityVolume Quantity_Liter(1e6);

const QuantityMass Quantity_Microgram(1e-9);
const QuantityMass Quantity_Milligram(1e-6);
const QuantityMass Quantity_Gram(0.001);
const QuantityMass Quantity_Kilogram(1.);
const QuantityMass Quantity_Ton(1000.);

const QuantityMass Quantity_Pound(0.45359237);
const QuantityMass Quantity_Ounce(0.0283495231);
const QuantityMass Quantity_Stone(6.35029318);
const QuantityMass Quantity_Hundredweights(50.80234544);

const QuantityTime Quantity_Second(1.);
const QuantityTime Quantity_Minute(60.);
const QuantityTime Quantity_Hour(3600.);

const QuantityAngle Quantity_Degree(1.);
const QuantityAngle Quantity_Radian(180. / 3.14159265358979323846);

} // namespace Mayo
