/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "qtcore_utils.h"

#include "../base/cpp_utils.h"

#include <algorithm>
#include <cassert>
#include <limits>

namespace Mayo::QtCoreUtils {

namespace {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using qtsize_t = qsizetype;
#else
using qtsize_t = int;
#endif

constexpr auto qtsize_t_MAX = std::numeric_limits<qtsize_t>::max();

} // namespace

QByteArray QByteArray_fromRawData(const QByteArray& bytes)
{
    return QByteArray::fromRawData(bytes.data(), bytes.size());
}

QByteArray QByteArray_fromRawData(std::string_view str)
{
    assert(Cpp::cmpLessEqual(str.size(), qtsize_t_MAX));
    return QByteArray::fromRawData(str.data(), static_cast<qtsize_t>(str.size()));
}

QByteArray toQByteArray(const char* data, size_t size)
{
    assert(Cpp::cmpLessEqual(size, qtsize_t_MAX));
    return QByteArray{ data, static_cast<qtsize_t>(size) };
}

std::vector<uint8_t> toStdByteArray(const QByteArray& bytes)
{
    std::vector<uint8_t> stdBytes;
    stdBytes.resize(bytes.size());
    std::copy(bytes.cbegin(), bytes.cend(), stdBytes.begin());
    return stdBytes;
}

Qt::CheckState toQtCheckState(Mayo::CheckState state)
{
    switch (state) {
    case CheckState::Off: return Qt::Unchecked;
    case CheckState::Partially: return Qt::PartiallyChecked;
    case CheckState::On: return Qt::Checked;
    }

    return Qt::Unchecked;
}

Mayo::CheckState toCheckState(Qt::CheckState state)
{
    switch (state) {
    case Qt::Unchecked: return CheckState::Off;
    case Qt::PartiallyChecked: return CheckState::Partially;
    case Qt::Checked: return CheckState::On;
    }

    return CheckState::Off;
}

} // namespace Mayo::QtCoreUtils
