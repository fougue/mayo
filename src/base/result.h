/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
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

    bool valid() const { return m_isValid; }
    operator bool() const { return m_isValid; }

    T& get();
    const T& get() const;

    const QString& errorText() const;

    static Result<T> ok(const T& data);
    static Result<T> ok(T&& data);
    static Result<T> error(const QString& errorText = QString());

private:
    // TODO: union
    T m_data;
    QString m_errorText;
    bool m_isValid;
};

template<> class Result<void> {
public:
    bool valid() const { return m_isValid; }
    operator bool() const { return m_isValid; }

    const QString& errorText() const;

    static const Result<void>& ok();
    static Result<void> error(const QString& errorText = QString());

private:
    Result<void>(bool valid);

    QString m_errorText;
    bool m_isValid;
};


// --
// -- Implementation
// --

template<typename T> T& Result<T>::get()
{
    if (m_isValid)
        return m_data;
    throw std::runtime_error("Data is invalid");
}

template<typename T> const T& Result<T>::get() const
{
    if (m_isValid)
        return m_data;
    throw std::runtime_error("Data is invalid");
}

template<typename T> const QString& Result<T>::errorText() const
{
    if (!m_isValid)
        return m_errorText;
    static const QString defaultErrorText;
    return defaultErrorText;
}

template<typename T> Result<T> Result<T>::error(const QString& errorText)
{
    Result<T> res;
    res.m_isValid = false;
    res.m_errorText = errorText;
    return std::move(res);
}

template<typename T> Result<T> Result<T>::ok(const T& data)
{
    Result<T> res;
    res.m_isValid = true;
    res.m_data = data;
    return std::move(res);
}

template<typename T> Result<T> Result<T>::ok(T&& data)
{
    Result<T> res;
    res.m_isValid = true;
    res.m_data = std::move(data);
    return std::move(res);
}

} // namespace Mayo
