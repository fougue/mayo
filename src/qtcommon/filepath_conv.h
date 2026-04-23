/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath_conv.h"

#include <QtCore/QString>
#include <QtCore/QFileInfo>

namespace Mayo {

// Returns a FilePath object constructed from input
inline FilePath filepathFrom(const QByteArray& bytes) { return filepathFrom(std::string_view{ bytes.constData() }); }
inline FilePath filepathFrom(const QString& str) { return reinterpret_cast<const char16_t*>(str.utf16()); }
inline FilePath filepathFrom(const QFileInfo& fi) { return filepathFrom(fi.filePath()); }

// FilePath -> QByteArray
template<> struct FilePathConv<QByteArray> {
    static auto to(const FilePath& fp) { return QByteArray::fromStdString(fp.u8string()); }
};

// FilePath -> QString
template<> struct FilePathConv<QString> {
    static auto to(const FilePath& fp) { return QString::fromStdString(fp.u8string()); }
};

// FilePath -> QFileInfo
template<> struct FilePathConv<QFileInfo> {
    static auto to(const FilePath& fp) { return QFileInfo(FilePathConv<QString>::to(fp)); }
};

} // namespace Mayo
