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

struct RecentFile {
    FilePath filepath;
    QPixmap thumbnail;
    int64_t thumbnailTimestamp = 0;
    bool recordThumbnail(GuiDocument* guiDoc, QSize size);
    bool isThumbnailOutOfSync() const;
};

using RecentFiles = std::vector<RecentFile>;
using PropertyRecentFiles = GenericProperty<RecentFiles>;

bool operator==(const RecentFile& lhs, const RecentFile& rhs);
QDataStream& operator<<(QDataStream& stream, const RecentFile& recentFile);
QDataStream& operator>>(QDataStream& stream, RecentFile& recentFile);
QDataStream& operator<<(QDataStream& stream, const RecentFiles& recentFiles);
QDataStream& operator>>(QDataStream& stream, RecentFiles& recentFiles);

} // namespace Mayo
