/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QString>

namespace Mayo {

template<typename T> struct Result {
    using Data = T;
    Data data;
    bool isValid;
    QString errorText;

    static Result<T> makeError(const QString& errorText = QString());
    static Result<T> makeValid(T&& data);
};




// --
// -- Implementation
// --

template<typename T>
Result<T> Result<T>::makeError(const QString& errorText) {
    Result<T> res;
    res.isValid = false;
    res.errorText = errorText;
    return std::move(res);
}

template<typename T>
Result<T> Result<T>::makeValid(T&& data) {
    Result<T> res;
    res.isValid = true;
    res.data = std::move(data);
    return std::move(res);
}

} // namespace Mayo
