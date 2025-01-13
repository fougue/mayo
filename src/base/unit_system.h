/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "quantity.h"
#include <string_view>

namespace Mayo {

class UnitSystem {
public:
    enum Schema { SI, ImperialUK };

    struct TranslateResult {
        double value;
        const char* strUnit; // UTF8
        double factor;
        constexpr operator double() const { return this->value; }
        constexpr operator bool() const { return this->strUnit != nullptr; }
    };

    template<Unit U>
    static TranslateResult translate(Schema schema, Quantity<U> qty) {
        return UnitSystem::translate(schema, qty.value(), U);
    }
    static TranslateResult translate(Schema schema, double value, Unit unit);
    static TranslateResult parseQuantity(std::string_view strQuantity, Unit* ptrUnit = nullptr);

    static TranslateResult translateLength(QuantityLength length, LengthUnit unit);
    static TranslateResult translateArea(QuantityArea area, AreaUnit unit);
    static TranslateResult translateVolume(QuantityVolume volume, VolumeUnit unit);
    static TranslateResult translateAngle(QuantityAngle angle, AngleUnit unit);

    static TranslateResult radians(QuantityAngle angle);
    static TranslateResult degrees(QuantityAngle angle);
    static TranslateResult meters(QuantityLength length);
    static TranslateResult millimeters(QuantityLength length);
    static TranslateResult squareMillimeters(QuantityArea area);
    static TranslateResult cubicMillimeters(QuantityVolume volume);
    static TranslateResult millimetersPerSecond(QuantityVelocity speed);
    static TranslateResult milliseconds(QuantityTime duration);
    static TranslateResult seconds(QuantityTime duration);

private:
    UnitSystem() = default;
};

} // namespace Mayo
