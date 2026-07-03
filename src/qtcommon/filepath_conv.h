/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath_conv.h"

#include <QtCore/QString>
#include <QtCore/QFileInfo>

namespace Mayo {

// QByteArray -> FilePath
inline FilePath filepathFrom(const QByteArray& bytes) {
    return filepathFrom(std::string_view{ bytes.constData() });
}

// QString -> FilePath
inline FilePath filepathFrom(const QString& str) {
    static_assert(sizeof(char16_t) == sizeof(ushort));
    static_assert(alignof(char16_t) == alignof(ushort));
    const auto ptr = reinterpret_cast<const char16_t*>(str.utf16()); // NOSONAR
    return std::filesystem::path{ptr, ptr + str.size()};
}

// QFileInfo -> FilePath
inline FilePath filepathFrom(const QFileInfo& fi) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return fi.filesystemFilePath();
#else
    return filepathFrom(fi.filePath());
#endif
}

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
