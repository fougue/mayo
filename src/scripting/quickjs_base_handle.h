/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <utility>

namespace Mayo {

template<typename T, typename Traits>
class QuickJsBaseHandle {
public:
    QuickJsBaseHandle() = default;

    explicit QuickJsBaseHandle(T value, Traits traits = {})
        : m_value(value),
          m_traits(std::move(traits))
    {
    }

    ~QuickJsBaseHandle()
    {
        this->reset();
    }

    QuickJsBaseHandle(const QuickJsBaseHandle&) = delete;
    QuickJsBaseHandle& operator=(const QuickJsBaseHandle&) = delete;

    QuickJsBaseHandle(QuickJsBaseHandle&& other) noexcept
        : m_value(std::exchange(other.m_value, Traits::empty())),
        m_traits(std::move(other.m_traits))
    {
    }

    QuickJsBaseHandle& operator=(QuickJsBaseHandle&& other) noexcept
    {
        if (this != &other) {
            this->reset();
            m_value = std::exchange(other.m_value, Traits::empty());
            m_traits = std::move(other.m_traits);
        }

        return *this;
    }

    T get() const
    {
        return m_value;
    }

    T release()
    {
        return std::exchange(m_value, Traits::empty());
    }

    void reset(T value = Traits::empty())
    {
        if (!m_traits.isEmpty(m_value))
            m_traits.destroy(m_value);

        m_value = value;
    }

    explicit operator bool() const
    {
        return !m_traits.isEmpty(m_value);
    }

protected:
    Traits& traits() { return m_traits; }
    const Traits& traits() const { return m_traits; }

private:
    T m_value = Traits::empty();
    Traits m_traits{};
};

} // namespace Mayo
