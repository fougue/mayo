/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "result.h"

namespace Mayo {

const QString& Result<void>::errorText() const
{
    if (!m_isValid)
        return m_errorText;
    static const QString defaultErrorText;
    return defaultErrorText;
}

const Result<void>& Result<void>::ok()
{
    static const Result<void> res(true);
    return res;
}

Result<void> Result<void>::error(const QString& errorText)
{
    Result<void> res(false);
    res.m_errorText = errorText;
    return std::move(res);
}

Result<void>::Result(bool valid)
    : m_isValid(valid)
{
}

} // namespace Mayo
