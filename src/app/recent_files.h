/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
