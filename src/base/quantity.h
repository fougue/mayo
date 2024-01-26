/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "math_utils.h"
#include "unit.h"

namespace Mayo {

// Numerical value of a unit of measurement
template<Unit U> class Quantity {
    using Qty = Quantity<U>;
public:
    constexpr Quantity() = default;
    constexpr explicit Quantity(double value): m_value(value) {}

    constexpr double value() const { return m_value; }
    constexpr void setValue(double v) { m_value = v; }
    constexpr Unit unit() const { return U; }

    constexpr Qty operator/(double v) const { return Qty(m_value / v); }
    constexpr Qty operator+(Qty q) const { return Qty(m_value + q.value()); }
    constexpr Qty operator-(Qty q) const { return Qty(m_value - q.value()); }
    constexpr Qty operator-() const { return Qty(-m_value); }

    constexpr bool operator==(Qty q) const { return m_value == q.value(); }
    constexpr bool operator<(Qty q) const { return m_value < q.value(); }
    constexpr bool operator>(Qty q) const { return m_value > q.value(); }
    constexpr bool operator<=(Qty q) const { return m_value <= q.value(); }
    constexpr bool operator>=(Qty q) const { return m_value >= q.value(); }

    constexpr Qty& operator+=(Qty q) { return this->change(m_value + q.value()); }
    constexpr Qty& operator-=(Qty q) { return this->change(m_value - q.value()); }
    //constexpr Qty& operator=(Qty q) { return this->change(q.value()); }

    constexpr static Qty null() { return Qty(0.); }

private:
    constexpr Qty& change(double v) {
        m_value = v;
        return *this;
    }

    double m_value;
};

template<Unit U> constexpr Quantity<U> operator*(Quantity<U> lhs, double rhs);
template<Unit U> constexpr Quantity<U> operator*(double lhs, Quantity<U> rhs);
template<Unit U> constexpr double operator/(Quantity<U> lhs, Quantity<U> rhs);

// Type aliases for most used quantity types in Mayo
using QuantityLength = Quantity<Unit::Length>;
using QuantityArea = Quantity<Unit::Area>;
using QuantityVolume = Quantity<Unit::Volume>;
using QuantityMass = Quantity<Unit::Mass>;
using QuantityTime = Quantity<Unit::Time>;
using QuantityAngle = Quantity<Unit::Angle>;
using QuantityVelocity = Quantity<Unit::Velocity>;
using QuantityDensity = Quantity<Unit::Density>;

constexpr QuantityArea operator*(QuantityLength lhs, QuantityLength rhs);
constexpr QuantityVolume operator*(QuantityLength lhs, QuantityArea rhs);
constexpr QuantityVolume operator*(QuantityArea lhs, QuantityLength rhs);
constexpr QuantityVelocity operator/(QuantityLength lhs, QuantityTime rhs);
constexpr QuantityTime operator/(QuantityLength lhs, QuantityVelocity rhs);

constexpr QuantityLength Quantity_Nanometer(1e-6);
constexpr QuantityLength Quantity_Micrometer(1e-3);
constexpr QuantityLength Quantity_Millimeter(1.);
constexpr QuantityLength Quantity_Centimeter(10.);
constexpr QuantityLength Quantity_Decimeter(100.);
constexpr QuantityLength Quantity_Meter(1000.);
constexpr QuantityLength Quantity_Kilometer(1e6);
constexpr QuantityLength Quantity_NauticalMile(1852 * Quantity_Kilometer.value());

constexpr QuantityArea Quantity_SquareMillimeter(1.);
constexpr QuantityArea Quantity_SquareCentimeter(100.);
constexpr QuantityArea Quantity_SquareMeter(1e6);
constexpr QuantityArea Quantity_SquareKilometer(1e12);

constexpr QuantityVolume Quantity_CubicMillimeter(1.);
constexpr QuantityVolume Quantity_CubicCentimeter(1000.);
constexpr QuantityVolume Quantity_CubicMeter(1e9);

constexpr QuantityLength Quantity_Thou(0.0254);
constexpr QuantityLength Quantity_Inch(25.4);
constexpr QuantityLength Quantity_Link(201.168);
constexpr QuantityLength Quantity_Foot(304.8);
constexpr QuantityLength Quantity_Yard(914.4);
constexpr QuantityLength Quantity_Rod(5029.2);
constexpr QuantityLength Quantity_Chain(20116.8);
constexpr QuantityLength Quantity_Furlong(201168);
constexpr QuantityLength Quantity_Mile(1609344);
constexpr QuantityLength Quantity_League(4828032);

constexpr QuantityArea Quantity_SquareInch(654.16);
constexpr QuantityArea Quantity_SquareFoot(92903.04);
constexpr QuantityArea Quantity_SquareYard(836127.36);
constexpr QuantityArea Quantity_SquareMile(Quantity_Mile.value() * Quantity_Mile.value());

constexpr QuantityVolume Quantity_CubicInch(16387.064);
constexpr QuantityVolume Quantity_CubicFoot(Quantity_Foot.value() * Quantity_SquareFoot.value());

constexpr QuantityVelocity Quantity_MillimeterPerSecond(1.);

constexpr QuantityVolume Quantity_Liter(1e6);
constexpr QuantityVolume Quantity_ImperialGallon(1e6 * 4.54609);
constexpr QuantityVolume Quantity_USGallon(1e6 * 3.785411784);

constexpr QuantityMass Quantity_Microgram(1e-9);
constexpr QuantityMass Quantity_Milligram(1e-6);
constexpr QuantityMass Quantity_Gram(0.001);
constexpr QuantityMass Quantity_Kilogram(1.);
constexpr QuantityMass Quantity_Ton(1000.);

constexpr QuantityDensity Quantity_GramPerCubicMillimeter(1e6);
constexpr QuantityDensity Quantity_GramPerCubicCentimeter(1000.);
constexpr QuantityDensity Quantity_GramPerCubicMeter(0.001);
constexpr QuantityDensity Quantity_KilogramPerCubicMeter(1.);

constexpr QuantityMass Quantity_Pound(0.45359237);
constexpr QuantityMass Quantity_Ounce(0.0283495231);
constexpr QuantityMass Quantity_Stone(6.35029318);
constexpr QuantityMass Quantity_Hundredweights(50.80234544);

constexpr QuantityTime Quantity_Millisecond(1e-3);
constexpr QuantityTime Quantity_Second(1.);
constexpr QuantityTime Quantity_Minute(60.);
constexpr QuantityTime Quantity_Hour(3600.);

constexpr QuantityAngle Quantity_Degree(3.14159265358979323846 / 180.);
constexpr QuantityAngle Quantity_Radian(1.);

namespace MathUtils {

template<Unit U>
bool fuzzyIsNull(Quantity<U> qty) { return fuzzyIsNull(qty.value()); }

template<Unit U>
bool fuzzyEqual(Quantity<U> lhs, Quantity<U> rhs) { return fuzzyEqual(lhs.value(), rhs.value()); }

} // namespace MathUtils



// --
// -- Implementation
// --

template<Unit U> constexpr Quantity<U> operator*(Quantity<U> lhs, double rhs) {
    return Quantity<U>(lhs.value() * rhs);
}

template<Unit U> constexpr Quantity<U> operator*(double lhs, Quantity<U> rhs) {
    return rhs * lhs;
}

template<Unit U> constexpr double operator/(Quantity<U> lhs, Quantity<U> rhs) {
    return lhs.value() / rhs.value();
}

constexpr QuantityArea operator*(QuantityLength lhs, QuantityLength rhs) {
    return QuantityArea(lhs.value() * rhs.value());
}

constexpr QuantityVolume operator*(QuantityLength lhs, QuantityArea rhs) {
    return QuantityVolume(lhs.value() * rhs.value());
}

constexpr QuantityVolume operator*(QuantityArea lhs, QuantityLength rhs) {
    return QuantityVolume(lhs.value() * rhs.value());
}

constexpr QuantityVelocity operator/(QuantityLength lhs, QuantityTime rhs) {
    return QuantityVelocity(lhs.value() / rhs.value());
}

constexpr QuantityTime operator/(QuantityLength lhs, QuantityVelocity rhs) {
    return QuantityTime(lhs.value() / rhs.value());
}

} // namespace Mayo
