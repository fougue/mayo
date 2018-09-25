/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "quantity.h"

namespace Mayo {

QuantityArea operator*(const QuantityLength &lhs, const QuantityLength &rhs) {
    return QuantityArea(lhs.value() * rhs.value());
}

QuantityVolume operator*(const QuantityLength &lhs, const QuantityArea &rhs) {
    return QuantityVolume(lhs.value() * rhs.value());
}

QuantityVolume operator*(const QuantityArea &lhs, const QuantityLength &rhs) {
    return QuantityVolume(lhs.value() * rhs.value());
}

QuantityVelocity operator/(const QuantityLength &lhs, const QuantityTime &rhs) {
    return QuantityVelocity(lhs.value() / rhs.value());
}

QuantityTime operator/(const QuantityLength& lhs, const QuantityVelocity& rhs) {
    return QuantityTime(lhs.value() / rhs.value());
}

} // namespace Mayo
