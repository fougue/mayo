/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/property_builtins.h"

#include <QtGui/QPixmap>
#include <vector>
class QDataStream;

namespace Mayo {

class GuiDocument;

// Provides information about a "recently" opened file
struct RecentFile {
    FilePath filepath;
    QPixmap thumbnail;
    int64_t thumbnailTimestamp = 0;
    bool recordThumbnail(GuiDocument* guiDoc, QSize size);
    bool isThumbnailOutOfSync() const;
    static int64_t timestampLastModified(const FilePath& fp);
};

// Alias for "array of RecentFile objects"
using RecentFiles = std::vector<RecentFile>;

// Alias for Property type owning an array of RecentFile objects
// This is useful to store recent files into application settings
using PropertyRecentFiles = GenericProperty<RecentFiles>;

bool operator==(const RecentFile& lhs, const RecentFile& rhs);

// Writes a RecentFile object to QDataStream
QDataStream& operator<<(QDataStream& stream, const RecentFile& recentFile);

// Writes array of RecentFile objects to QDataStream
QDataStream& operator<<(QDataStream& stream, const RecentFiles& recentFiles);

// Extracts a RecentFile object from QDataStream
QDataStream& operator>>(QDataStream& stream, RecentFile& recentFile);

// Extracts array of RecentFile objects from QDataStream
QDataStream& operator>>(QDataStream& stream, RecentFiles& recentFiles);

} // namespace Mayo
