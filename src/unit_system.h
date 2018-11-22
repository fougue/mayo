/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "quantity.h"
#include <string>

namespace Mayo {

class UnitSystem {
public:
    enum Schema {
        SI,
        ImperialUK
    };

    struct TranslateResult {
        double value;
        const char* strUnit; // UTF8
        double factor;
        operator double() const { return this->value; }
    };

    template<Unit UNIT>
    static TranslateResult translate(Schema schema, const Quantity<UNIT>& qty) {
        return UnitSystem::translate(schema, qty.value(), UNIT);
    }
    static TranslateResult translate(Schema schema, double value, Unit unit);

    static TranslateResult radians(QuantityAngle angle);
    static TranslateResult degrees(QuantityAngle angle);
    static TranslateResult millimeters(QuantityLength length);
    static TranslateResult millimetersPerSecond(QuantityVelocity speed);

    static std::string toSystemLocaleString(double value, const char* strUnit);
    static std::string toCLocaleString(double value, const char* strUnit);

private:
    UnitSystem() = default;
};

} // namespace Mayo
