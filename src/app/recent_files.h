/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/property_builtins.h"

#include <QtCore/QByteArray>

#include <vector>

namespace Mayo {

class GuiDocument;

struct Thumbnail {
    QByteArray imageData; // PNG
    int64_t imageCacheKey = -1;
};

// Provides information about a "recently" opened file
struct RecentFile {
    FilePath filepath;
    Thumbnail thumbnail;
    int64_t thumbnailTimestamp = 0;
    bool isThumbnailOutOfSync() const;
    static int64_t timestampLastModified(const FilePath& fp);
};

// Alias for "array of RecentFile objects"
using RecentFiles = std::vector<RecentFile>;

// Alias for Property type owning an array of RecentFile objects
// This is useful to store recent files into application settings
using PropertyRecentFiles = GenericProperty<RecentFiles>;

bool operator==(const RecentFile& lhs, const RecentFile& rhs);

} // namespace Mayo
