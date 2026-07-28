/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/property_builtins.h"

#include <QtCore/QByteArray>
#include <QtCore/QSize>

#include <functional>
#include <vector>

class QDataStream;

namespace Mayo {

class GuiApplication;
class GuiDocument;

// Thumbnail(image) associated with a recent file
struct Thumbnail {
    // PNG-encoded data
    QByteArray imageData;

    // Cache key identifying the image
    // A negative value indicates that no cache entry is available
    int64_t imageCacheKey = -1;
};

// Information about a "recently" opened file
struct RecentFile {
    // Path to the recent file
    FilePath filepath;

    // Cached thumbnail associated with the recent file
    Thumbnail thumbnail;

    // Last modification timestamp of the file when the thumbnail was generated
    int64_t thumbnailTimestamp = 0;

    // Returns whether the cached thumbnail is out of sync with the current file
    bool isThumbnailOutOfSync() const;

    // Returns the last modification timestamp of the given file
    static int64_t timestampLastModified(const FilePath& fp);
};

// Returns whether both RecentFile objects are equal
bool operator==(const RecentFile& lhs, const RecentFile& rhs);

// Alias for "array of RecentFile objects"
using RecentFiles = std::vector<RecentFile>;

// Provides binary serialization helpers for RecentFile and RecentFiles using QDataStream
struct RecentFileIO {
    static void read(QDataStream& stream, RecentFile* recentFile);
    static void read(QDataStream& stream, RecentFiles* recentFiles);
    static void write(QDataStream& stream, const RecentFiles& recentFiles);
};

// Property type owning an array of RecentFile objects
// This is useful to store recent files into application settings
class PropertyRecentFiles : public GenericProperty<RecentFiles> {
public:
    // Inherit GenericProperty<RecentFiles> constructors
    using GenericProperty<RecentFiles>::GenericProperty;

    // Maximum number of recent files to keep
    unsigned countLimit() const;
    void setCountLimit(unsigned limit);

    // Size of generated thumbnails
    QSize thumbnailSize() const;
    void setThumbnailSize(int w, int h);

    // Function used to generate a thumbnail for a document
    using ThumbnailGeneratorFunction = std::function<Thumbnail(GuiDocument*, QSize)>;
    const ThumbnailGeneratorFunction& thumbnailGenerator() const;
    void setThumbnailGenerator(ThumbnailGeneratorFunction fn);

    // Inserts the specified file at the beginning of the recent files list
    // If the file already exists in the list, it is moved to the front
    void prepend(const FilePath& fp);

    // Finds the recent file entry matching the given file path
    // Returns nullptr if no matching entry is found
    const RecentFile* find(const FilePath& fp) const;

    // Records the specified document in the recent files list
    // Updates or creates the corresponding entry and refreshes its thumbnail
    void record(GuiDocument* guiDoc);
    void record(GuiApplication* guiApp);

private:
    unsigned m_countLimit = -1;
    QSize m_thumbnailSize{190, 150};
    ThumbnailGeneratorFunction m_fnThumbnailGenerator;
};

} // namespace Mayo
