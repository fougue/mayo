/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// Workaround bug https://bugreports.qt.io/browse/QTBUG-73263 which affects GCC builds
#ifndef Q_MOC_RUN
#  include <filesystem>
#endif

namespace Mayo {

using FilePath = std::filesystem::path;

} // namespace Mayo


#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <type_traits>

namespace Mayo {

inline FilePath filepathFrom(const QByteArray& bytes) { return bytes.toStdString(); }
inline FilePath filepathFrom(const QString& str) { return str.toStdString(); }
inline FilePath filepathFrom(const QFileInfo& fi) { return filepathFrom(fi.filePath()); }

template<typename T> T filepathTo(const FilePath& fp)
{
    if constexpr(std::is_same<T, QByteArray>::value) {
        return QByteArray::fromStdString(fp.u8string());
    }
    if constexpr(std::is_same<T, QString>::value) {
        return QString::fromStdString(fp.u8string());
    }
    else if constexpr(std::is_same<T, QFileInfo>::value) {
        return QFileInfo(filepathTo<QString>(fp));
    }
}

} // namespace Mayo
