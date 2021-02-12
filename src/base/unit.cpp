/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "unit.h"

namespace Mayo {

#if 0
static const char* BaseUnit_symbol(BaseUnit baseUnit)
{
    switch (baseUnit) {
    case BaseUnit::Length: return "m";
    case BaseUnit::Mass: return "kg";
    case BaseUnit::Time: return "s";
    case BaseUnit::ElectricCurrent: return "A";
    case BaseUnit::ThermodynamicTemperature: return "K";
    case BaseUnit::AmountOfSubstance: return "mol";
    case BaseUnit::LuminousIntensity: return "cd";
    case BaseUnit::Angle: return "°";
    }
    return "";
}

static constexpr size_t Unit_powIndex(BaseUnit baseUnit) {
    return static_cast<size_t>(baseUnit);
}

Unit operator*(const Unit& unit, BaseUnit baseUnit)
{
    Unit out = unit.m_powBaseUnit;
    ++out.m_powBaseUnit.at(Unit_powIndex(baseUnit));
    out.buildSymbol();
    return out;
}

Unit operator*(const Unit& lhs, const Unit& rhs)
{
    Unit out = lhs.m_powBaseUnit;
    for (size_t i = 0; i < lhs.m_powBaseUnit.size(); ++i)
        out.m_powBaseUnit.at(i) += rhs.m_powBaseUnit.at(i);
    out.buildSymbol();
    return out;
}

Unit operator/(const Unit& unit, BaseUnit baseUnit)
{
    Unit out = unit.m_powBaseUnit;
    --out.m_powBaseUnit.at(Unit_powIndex(baseUnit));
    out.buildSymbol();
    return out;
}

Unit operator/(const Unit& lhs, const Unit& rhs)
{
    Unit out = lhs.m_powBaseUnit;
    for (size_t i = 0; i < lhs.m_powBaseUnit.size(); ++i)
        out.m_powBaseUnit.at(i) -= rhs.m_powBaseUnit.at(i);
    out.buildSymbol();
    return out;
}

const Unit Unit::None;

const Unit Unit::Length = BaseUnit::Length;
const Unit Unit::Mass = BaseUnit::Mass;
const Unit Unit::Angle = BaseUnit::Angle;
const Unit Unit::Time = BaseUnit::Time;
const Unit Unit::Area = Unit::Length * Unit::Length;
const Unit Unit::Volume = Unit::Area * Unit::Length;
const Unit Unit::Velocity = Unit::Length / BaseUnit::Time;
const Unit Unit::Acceleration = Unit::Length / (Unit::Time * Unit::Time);
const Unit Unit::Density = Unit::Mass / Unit::Volume;
const Unit Unit::Pressure = Unit::Mass / (Unit::Length * Unit::Time * Unit::Time);

const std::string &Unit::symbol() const
{
    return m_symbol;
}

Unit::Unit(BaseUnit base)
{
    m_powBaseUnit.at(Unit_powIndex(base)) = 1;
    this->buildSymbol();
}

Unit::Unit(const Unit::ArrayPowBaseUnit &arrayPow)
    : m_powBaseUnit(arrayPow)
{
}

Unit &Unit::operator=(BaseUnit base)
{
    m_powBaseUnit.at(Unit_powIndex(base)) = 1;
    this->buildSymbol();
    return *this;
}

void Unit::buildSymbol()
{
    m_symbol.clear();
    int numCount = 0;
    int divCount = 0;
    std::stringstream numStream;
    std::stringstream divStream;
    for (size_t i = 0; i < m_powBaseUnit.size(); ++i) {
        const int8_t powBaseUnit = m_powBaseUnit.at(i);
        if (powBaseUnit != 0) {
            const int8_t absPowBaseUnit = std::abs(powBaseUnit);
            std::stringstream& refStream = powBaseUnit > 0 ? numStream : divStream;
            int& refCount = powBaseUnit > 0 ? numCount : divCount;
            if (refCount != 0)
                refStream << '.';
            refStream << BaseUnit_symbol(static_cast<BaseUnit>(i));
            if (absPowBaseUnit == 2)
                refStream << "\xc2\xb2";
            else if (absPowBaseUnit == 3)
                refStream << "\xc2\xb3";
            else if (absPowBaseUnit != 1)
                refStream << '^' << powBaseUnit;
            ++refCount;
        }
    }
    if (numCount > 0) {
        m_symbol = numStream.str();
        if (divCount > 0)
            m_symbol += "/" + divStream.str();
    }
}
#endif

} // namespace Mayo
