/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <type_traits>

namespace Mayo {

template<typename Scalar, typename Tag>
class TypedScalar {
public:
    using ScalarType = Scalar;
    static_assert(std::is_scalar<Scalar>::value, "Type T is not scalar");

    TypedScalar() = default;
    explicit TypedScalar(Scalar scalar) : m_scalar(scalar) {}

    Scalar get() const { return m_scalar; }

    bool operator==(const TypedScalar<Scalar, Tag>& other) const {
        return m_scalar == other.m_scalar;
    }

    bool operator!=(const TypedScalar<Scalar, Tag>& other) const {
        return m_scalar != other.m_scalar;
    }

private:
    Scalar m_scalar;
};

} // namespace Mayo
