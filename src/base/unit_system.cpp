/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "unit_system.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QtGlobal>
#include <QtCore/QLocale>
#include <cfloat>

#define MAYO_CUBIC_SYMBOL "\xc2\xb3"

namespace Mayo {

namespace Internal {

static const char* symbol(Unit unit)
{
    switch (unit) {
    case Unit::None: return "";
    // Base
    case Unit::Length: return "m";
    case Unit::Mass: return "kg";
    case Unit::Time: return "s";
    case Unit::ElectricCurrent: return "A";
    case Unit::ThermodynamicTemperature: return "K";
    case Unit::AmountOfSubstance: return "mol";
    case Unit::LuminousIntensity: return "cd";
    case Unit::Angle: return "rad";
    // Derived
    case Unit::Area: return "m²";
    case Unit::Volume: return "m" MAYO_CUBIC_SYMBOL;
    case Unit::Velocity: return "m/s";
    case Unit::Acceleration: return "m/s²";
    case Unit::Density: return "kg/m" MAYO_CUBIC_SYMBOL;
    case Unit::Pressure: return "kg/m.s²";
    }

    return "?";
}

struct Threshold_UnitInfo {
    double threshold;
    const char* str;
    double factor;
};

template<size_t N> UnitSystem::TranslateResult translate(
        double value, const Threshold_UnitInfo (&array)[N])
{
    for (const Threshold_UnitInfo& t : array) {
        if (value < t.threshold)
            return { value / t.factor, t.str, t.factor };
    }

    return { value, nullptr, 1. };
}

static UnitSystem::TranslateResult translateSI(double value, Unit unit)
{
    if (unit == Unit::Length)
        return { value, "mm", 1. };
    else if (unit == Unit::Area)
        return { value, "mm²", 1. };
    else if (unit == Unit::Volume)
        return { value, "mm" MAYO_CUBIC_SYMBOL, 1. };
    else if (unit == Unit::Velocity)
        return { value, "mm/s", 1. };
    else if (unit == Unit::Density)
        return { value, "kg/mm" MAYO_CUBIC_SYMBOL, 1. };
    else if (unit == Unit::Pressure)
        return { value, "kPa", 1. };

    return { value, symbol(unit), 1. };
}

static UnitSystem::TranslateResult translateImperialUK(double value, Unit unit)
{
    if (unit == Unit::Length)
        return { value / 25.4, "in", 25.4 };
    else if (unit == Unit::Area)
        return { value / 645.16, "in²", 654.16 };
    else if (unit == Unit::Volume)
        return { value / 16387.064, "in" MAYO_CUBIC_SYMBOL, 16387.064 };
    else if (unit == Unit::Velocity)
        return { value / (25.4 / 60.), "in/min", 25.4 / 60. };

    return { value, symbol(unit), 1. };
}

static UnitSystem::TranslateResult translateSI_ranged(double value, Unit unit)
{
    if (unit == Unit::Length) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 1e-9, "m", 1000. },   // < 0.001nm
            { 0.001, "nm", 1e-6 },  // < 1µm
            { 0.1, "µm", 0.001 },   // < 0.1mm
            { 100., "mm", 1. },     // < 10cm
            { 1e7, "m", 1000. },    // < 10km
            { 1e11, "km", 1e6 },    // < 100000km
            { DBL_MAX, "m", 1000. }
        };
        return Internal::translate(value, array);
    }
    else if (unit == Unit::Area) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 100., "mm²", 1. },      // < 1cm²
            { 1e12, "m²", 1e6 },      // < 1km²
            { DBL_MAX, "km²", 1e12 }  // > 1km²
        };
        return Internal::translate(value, array);
    }
    else if (unit == Unit::Volume) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 1e4, "mm^3", 1. },       // < 10cm^3
            { 1e18, "m^3", 1e9 },      // < 1km^3
            { DBL_MAX, "km^3", 1e18 }  // > 1km^3
        };
        return Internal::translate(value, array);
    }
    else if (unit == Unit::Density) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 1e-4, "kg/m^3", 1e-9 },
            { 1., "kg/cm^3", 0.001 },
            { DBL_MAX, "kg/mm^3", 1. }
        };
        return Internal::translate(value, array);
    }
    else if (unit == Unit::Pressure) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 10, "Pa", 0.001 },
            { 10000, "kPa", 1. },
            { 1e7, "MPa", 1000. },
            { 1e10, "GPa", 1e6 },
            { DBL_MAX, "Pa", 0.001 }  // > 1000Gpa
        };
        return Internal::translate(value, array);
    }
    else {
        // TODO
    }

    return { value, symbol(unit), 1. };
}

static UnitSystem::TranslateResult translateImperialUK_ranged(double value, Unit unit)
{
    if (unit == Unit::Length) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 0.00000254, "in", 25.4 }, // < 0.001 thou
            { 2.54, "thou", 0.0254 },   // < 0.1 in
            { 304.8, "\"", 25.4 },
            { 914.4, "'", 304.8 },
            { 1609344., "yd", 914.4 },
            { 1609344000, "mi", 1609344 },
            { DBL_MAX, "in", 25.4 } // > 1000 mi
        };

        return Internal::translate(value, array);
    }

    return translateImperialUK(value, unit);
}

static std::string toLocaleString(const QLocale& locale, double value, const char* strUnit)
{
    return QCoreApplication::translate("Mayo::UnitSystem", "%1%2")
            .arg(locale.toString(value), QString::fromUtf8(strUnit))
            .toStdString();
}

} // namespace Internal

std::string UnitSystem::toSystemLocaleString(double value, const char* strUnit)
{
    return Internal::toLocaleString(QLocale::system(), value, strUnit);
}

std::string UnitSystem::toCLocaleString(double value, const char* strUnit)
{
    return Internal::toLocaleString(QLocale::c(), value, strUnit);
}

UnitSystem::TranslateResult UnitSystem::translate(
        Schema schema, double value, Unit unit)
{
    switch (schema) {
    case Schema::SI:
        return Internal::translateSI(value, unit);
    case Schema::ImperialUK:
        return Internal::translateImperialUK(value, unit);
    }

    Q_UNREACHABLE();
    return {};
}

UnitSystem::TranslateResult UnitSystem::radians(QuantityAngle angle)
{
    return { angle.value(), "rad", 1. };
}

UnitSystem::TranslateResult UnitSystem::degrees(QuantityAngle angle)
{
    constexpr double factor = Quantity_Degree.value();
    const double rad = angle.value();
    return { rad / factor, "°", factor };
}

UnitSystem::TranslateResult UnitSystem::millimeters(QuantityLength length)
{
    return { length.value(), "mm", 1. };
}

UnitSystem::TranslateResult UnitSystem::cubicMillimeters(QuantityVolume volume)
{
    return { volume.value(), "mm" MAYO_CUBIC_SYMBOL, 1. };
}

UnitSystem::TranslateResult UnitSystem::millimetersPerSecond(QuantityVelocity speed)
{
    return { speed.value(), "mm/s", 1. };
}

UnitSystem::TranslateResult UnitSystem::seconds(QuantityTime duration)
{
    return { duration.value(), "s", 1. };
}

} // namespace Mayo
