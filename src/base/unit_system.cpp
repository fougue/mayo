/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "unit_system.h"

#include <fast_float/fast_float.h>
#include <cassert>

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
    case Unit::Volume: return "m³";
    case Unit::Velocity: return "m/s";
    case Unit::Acceleration: return "m/s²";
    case Unit::Density: return "kg/m³";
    case Unit::Pressure: return "kg/m.s²";
    }

    return "?";
}

struct UnitInfo {
    Unit unit;
    const char* str;
    double factor;
};
const UnitInfo arrayUnitInfo_SI[] = {
    //Length
    { Unit::Length, "mm", Quantity_Millimeter.value() },
    { Unit::Length, "cm", Quantity_Centimeter.value() },
    { Unit::Length, "dm", Quantity_Decimeter.value() },
    { Unit::Length, "m",  Quantity_Meter.value() },
    { Unit::Length, "km", Quantity_Kilometer.value() },
    { Unit::Length, "nm", Quantity_Nanometer.value() },
    { Unit::Length, "µm", Quantity_Micrometer.value() },
    // Angle
    { Unit::Angle, "rad", Quantity_Radian.value() },
    { Unit::Angle, "deg", Quantity_Degree.value() },
    { Unit::Angle, "°",   Quantity_Degree.value() },
    // Area
    { Unit::Area, "mm²", Quantity_SquareMillimeter.value() },
    { Unit::Area, "cm²", Quantity_SquareCentimeter.value() },
    { Unit::Area, "m²",  Quantity_SquareMeter.value() },
    { Unit::Area, "km²", Quantity_SquareKilometer.value() },
    // Volume
    { Unit::Volume, "mm³", Quantity_CubicMillimeter.value() },
    { Unit::Volume, "cm³", Quantity_CubicCentimeter.value() },
    { Unit::Volume, "m³",  Quantity_CubicMeter.value() },
    { Unit::Volume, "L",   Quantity_Liter.value() },
    // Velocity
    { Unit::Velocity, "mm/s", Quantity_MillimeterPerSecond.value() },
    // Density
    { Unit::Density, "kg/m³", 1 },
    { Unit::Density, "g/m³", 1000. },
    { Unit::Density, "g/cm³", 0.001 },
    { Unit::Density, "g/mm³", 1e-6 },
    // Pressure
    { Unit::Pressure, "kPa", 1. },
    { Unit::Pressure, "Pa", 0.001 },
    { Unit::Pressure, "kPa", 1. },
    { Unit::Pressure, "MPa", 1000. },
    { Unit::Pressure, "GPa", 1e6 }
};

const UnitInfo arrayUnitInfo_ImperialUK[] = {
    //Length
    { Unit::Length, "thou", Quantity_Thou.value() },
    { Unit::Length, "in", Quantity_Inch.value() },
    { Unit::Length, "\"", Quantity_Inch.value() },
    { Unit::Length, "ft", Quantity_Foot.value() },
    { Unit::Length, "'",  Quantity_Foot.value() },
    { Unit::Length, "yd", Quantity_Yard.value() },
    { Unit::Length, "mi", Quantity_Mile.value() },
    // Others
    { Unit::Area, "in²", Quantity_SquareInch.value() },
    { Unit::Area, "\"²", Quantity_SquareInch.value() },
    { Unit::Area, "ft²", Quantity_SquareFoot.value() },
    { Unit::Area, "'²",  Quantity_SquareFoot.value() },
    { Unit::Area, "yd²", Quantity_SquareYard.value() },
    { Unit::Volume, "in³", Quantity_CubicInch.value() },
    { Unit::Volume, "\"³", Quantity_CubicInch.value() },
    { Unit::Volume, "ft³", Quantity_CubicFoot.value() },
    { Unit::Volume, "'³",  Quantity_CubicFoot.value() },
    { Unit::Velocity, "in/min", Quantity_Inch.value() / 60. },
    { Unit::Velocity, "\"/min", Quantity_Inch.value() / 60. },
};

static UnitSystem::TranslateResult translateSI(double value, Unit unit)
{
    switch (unit) {
    case Unit::Length:
        return { value, "mm", 1. };
    case Unit::Area:
        return { value, "mm²", 1. };
    case Unit::Volume:
        return { value, "mm³", 1. };
    case Unit::Velocity:
        return { value, "mm/s", 1. };
    case Unit::Density:
        return { value, "kg/m³", 1. };
    case Unit::Pressure:
        return { value, "kPa", 1. };
    default:
        return { value, symbol(unit), 1. };
    }
}

static UnitSystem::TranslateResult translateImperialUK(double value, Unit unit)
{
    switch (unit) {
    case Unit::Length:
        return { value / 25.4, "in", 25.4 };
    case Unit::Area:
        return { value / 645.16, "in²", 654.16 };
    case Unit::Volume:
        return { value / 16387.064, "in³", 16387.064 };
    case Unit::Velocity:
        return { value / (25.4 / 60.), "in/min", 25.4 / 60. };
    default:
        return { value, symbol(unit), 1. };
    }
}

template<typename QuantityType>
UnitSystem::TranslateResult translateHelper(QuantityType qty, QuantityType qtyFactor, const char* strUnit)
{
    const double factor = qtyFactor.value();
    return { qty.value() / factor, strUnit, factor };
}


#if 0
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
            { 1e4, "mm³", 1. },       // < 10cm^3
            { 1e18, "m³", 1e9 },      // < 1km^3
            { DBL_MAX, "km³", 1e18 }  // > 1km^3
        };
        return Internal::translate(value, array);
    }
    else if (unit == Unit::Density) {
        static const Internal::Threshold_UnitInfo array[] = {
            { 1e-4, "kg/m³", 1e-9 },
            { 1., "kg/cm³", 0.001 },
            { DBL_MAX, "kg/mm³", 1. }
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
#endif

} // namespace Internal

UnitSystem::TranslateResult UnitSystem::translate(Schema schema, double value, Unit unit)
{
    switch (schema) {
    case Schema::SI:
        return Internal::translateSI(value, unit);
    case Schema::ImperialUK:
        return Internal::translateImperialUK(value, unit);
    }

    assert(false);
    return {};
}

UnitSystem::TranslateResult UnitSystem::parseQuantity(std::string_view strQuantity, Unit* ptrUnit)
{
    auto fnAssignUnit = [=](Unit unit) {
        if (ptrUnit)
            *ptrUnit = unit;
    };

    fnAssignUnit(Unit::None);

    double v;
    auto res = fast_float::from_chars(strQuantity.data(), strQuantity.data() + strQuantity.size(), v);
    if (res.ec != std::errc())
        return {};

    std::string_view strUnit = strQuantity.substr(res.ptr - strQuantity.data());
    if (strUnit.empty())
        return { v, nullptr, 1. };

    for (const Internal::UnitInfo& unitInfo : Internal::arrayUnitInfo_SI) {
        if (strUnit == unitInfo.str) {
            fnAssignUnit(unitInfo.unit);
            return { v, unitInfo.str, unitInfo.factor };
        }
    }

    for (const Internal::UnitInfo& unitInfo : Internal::arrayUnitInfo_ImperialUK) {
        if (strUnit == unitInfo.str) {
            fnAssignUnit(unitInfo.unit);
            return { v, unitInfo.str, unitInfo.factor };
        }
    }

    return {};
}

UnitSystem::TranslateResult UnitSystem::translateLength(QuantityLength length, LengthUnit unit)
{
    auto fnTrResult = [=](QuantityLength qtyFactor, const char* strUnit) {
        return Internal::translateHelper(length, qtyFactor, strUnit);
    };
    switch (unit) {
        // SI
    case LengthUnit::Nanometer:  return fnTrResult(Quantity_Nanometer, "nm");
    case LengthUnit::Micrometer: return fnTrResult(Quantity_Micrometer, "µm");
    case LengthUnit::Millimeter: return fnTrResult(Quantity_Millimeter, "mm");
    case LengthUnit::Centimeter: return fnTrResult(Quantity_Centimeter, "cm");
    case LengthUnit::Decimeter:  return fnTrResult(Quantity_Decimeter, "dm");
    case LengthUnit::Meter: return fnTrResult(Quantity_Meter, "m");
    case LengthUnit::Kilometer: return fnTrResult(Quantity_Kilometer, "km");
    case LengthUnit::NauticalMile: return fnTrResult(Quantity_NauticalMile, "nmi");
        // Imperial UK
    case LengthUnit::Thou: return fnTrResult(Quantity_Thou, "thou");
    case LengthUnit::Inch: return fnTrResult(Quantity_Inch, "in");
    case LengthUnit::Link: return fnTrResult(Quantity_Link, "link");
    case LengthUnit::Foot: return fnTrResult(Quantity_Foot, "ft");
    case LengthUnit::Yard: return fnTrResult(Quantity_Yard, "yd");
    case LengthUnit::Rod:  return fnTrResult(Quantity_Rod, "rod");
    case LengthUnit::Chain: return fnTrResult(Quantity_Chain, "chain");
    case LengthUnit::Furlong: return fnTrResult(Quantity_Furlong, "fur");
    case LengthUnit::Mile:    return fnTrResult(Quantity_Mile, "mi");
    case LengthUnit::League:  return fnTrResult(Quantity_League, "league");
    }
    return {};
}

UnitSystem::TranslateResult UnitSystem::translateArea(QuantityArea area, AreaUnit unit)
{
    auto fnTrResult = [=](QuantityArea qtyFactor, const char* strUnit) {
        return Internal::translateHelper(area, qtyFactor, strUnit);
    };
    switch (unit) {
        // SI
    case AreaUnit::SquareMillimeter:  return fnTrResult(Quantity_SquareMillimeter, "mm²");
    case AreaUnit::SquareCentimeter: return fnTrResult(Quantity_SquareCentimeter, "cm²");
    case AreaUnit::SquareMeter: return fnTrResult(Quantity_SquareMeter, "m²");
    case AreaUnit::SquareKilometer: return fnTrResult(Quantity_SquareKilometer, "km²");
        // Imperial UK
    case AreaUnit::SquareInch: return fnTrResult(Quantity_SquareInch, "in²");
    case AreaUnit::SquareFoot: return fnTrResult(Quantity_SquareFoot, "ft²");
    case AreaUnit::SquareYard: return fnTrResult(Quantity_SquareYard, "yd²");
    case AreaUnit::SquareMile: return fnTrResult(Quantity_SquareMile, "mi²");
    }
    return {};
}

UnitSystem::TranslateResult UnitSystem::translateVolume(QuantityVolume volume, VolumeUnit unit)
{
    auto fnTrResult = [=](QuantityVolume qtyFactor, const char* strUnit) {
        return Internal::translateHelper(volume, qtyFactor, strUnit);
    };
    switch (unit) {
        // SI
    case VolumeUnit::CubicMillimeter:  return fnTrResult(Quantity_CubicMillimeter, "mm³");
    case VolumeUnit::CubicCentimeter: return fnTrResult(Quantity_CubicCentimeter, "cm³");
    case VolumeUnit::CubicMeter: return fnTrResult(Quantity_CubicMeter, "m³");
        // Imperial UK
    case VolumeUnit::CubicInch: return fnTrResult(Quantity_CubicInch, "in³");
    case VolumeUnit::CubicFoot: return fnTrResult(Quantity_CubicFoot, "ft³");
        // Others
    case VolumeUnit::Liter: return fnTrResult(Quantity_Liter, "L");
    case VolumeUnit::ImperialGallon: return fnTrResult(Quantity_ImperialGallon, "GBgal");
    case VolumeUnit::USGallon: return fnTrResult(Quantity_USGallon, "USgal");
    }
    return {};
}

UnitSystem::TranslateResult UnitSystem::translateAngle(QuantityAngle angle, AngleUnit unit)
{
    switch (unit) {
    case AngleUnit::Radian: return UnitSystem::radians(angle);
    case AngleUnit::Degree: return UnitSystem::degrees(angle);
    }
    return {};
}

UnitSystem::TranslateResult UnitSystem::radians(QuantityAngle angle)
{
    return { angle.value(), "rad", 1. };
}

UnitSystem::TranslateResult UnitSystem::degrees(QuantityAngle angle)
{
    return Internal::translateHelper(angle, Quantity_Degree, "°");
}

UnitSystem::TranslateResult UnitSystem::meters(QuantityLength length)
{
    return Internal::translateHelper(length, Quantity_Meter, "m");
}

UnitSystem::TranslateResult UnitSystem::millimeters(QuantityLength length)
{
    return { length.value(), "mm", 1. };
}

UnitSystem::TranslateResult UnitSystem::squareMillimeters(QuantityArea area)
{
    return { area.value(), "mm²", 1. };
}

UnitSystem::TranslateResult UnitSystem::cubicMillimeters(QuantityVolume volume)
{
    return { volume.value(), "mm³", 1. };
}

UnitSystem::TranslateResult UnitSystem::millimetersPerSecond(QuantityVelocity speed)
{
    return { speed.value(), "mm/s", 1. };
}

UnitSystem::TranslateResult UnitSystem::milliseconds(QuantityTime duration)
{
    return Internal::translateHelper(duration, Quantity_Millisecond, "ms");
}

UnitSystem::TranslateResult UnitSystem::seconds(QuantityTime duration)
{
    return { duration.value(), "s", 1. };
}

} // namespace Mayo
