/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <exception>
#include <QtCore/QString>

namespace Mayo {

template<typename T> class Result {
public:
    using Data = T;

#if 0
    ~Result()
    {
        if (m_isValid)
            m_data.~T();
        else
            m_errorText.~QString();
    }

    Result(const T& rhs)
        : m_data(rhs),
          m_isValid(true)
    {}

    Result(T&& rhs)
        : m_data(std::move(rhs)),
          m_isValid(true)
    {}

    Result(const Result& rhs)
        : m_isValid(rhs.m_isValid)
    {
        if (m_isValid)
            new(&m_data) T(rhs.m_data);
        else
            new(&m_errorText) QString(rhs.m_errorText);
    }

    Result(Result&& rhs)
        : m_isValid(rhs.m_isValid)
    {
        if (m_isValid)
            new(&m_data) T(std::move(rhs.m_data));
        else
            new(&m_errorText) QString(std::move(rhs.m_errorText));
    }
#endif

    bool valid() const { return m_isValid; }
    operator bool() const { return m_isValid; }

    T& get()
    {
        if (m_isValid)
            return m_data;
        throw std::runtime_error("Data is invalid");
    }

    const T& get() const
    {
        if (m_isValid)
            return m_data;
        throw std::runtime_error("Data is invalid");
    }

    const QString& errorText() const
    {
        if (!m_isValid)
            return m_errorText;
        static const QString defaultErrorText;
        return defaultErrorText;
    }

    static Result<T> makeValid(T&& data);
    static Result<T> makeError(const QString& errorText = QString());

private:
#if 0
    Result() {}

    union {
        T m_data;
        QString m_errorText;
    };
#else
    T m_data;
    QString m_errorText;
#endif

    bool m_isValid;
};




// --
// -- Implementation
// --

template<typename T>
Result<T> Result<T>::makeError(const QString& errorText) {
    Result<T> res;
    res.m_isValid = false;
    res.m_errorText = errorText;
    return std::move(res);
}

template<typename T>
Result<T> Result<T>::makeValid(T&& data) {
    Result<T> res;
    res.m_isValid = true;
    res.m_data = std::move(data);
    return std::move(res);
}

} // namespace Mayo
