/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <type_traits>

namespace Mayo {

template<typename SCALAR, typename TAG>
class TypedScalar {
public:
    using ScalarType = SCALAR;

    static_assert(std::is_scalar<SCALAR>::value, "Type T is not scalar");

    TypedScalar() = default;
    explicit TypedScalar(SCALAR scalar) : m_scalar(scalar) {}

    SCALAR get() const { return m_scalar; }

private:
    SCALAR m_scalar;
};

} // namespace Mayo
