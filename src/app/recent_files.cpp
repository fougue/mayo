/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_files.h"

#include <fmt/format.h>
#include <QtCore/QtDebug>

namespace Mayo {

bool RecentFile::isThumbnailOutOfSync() const
{
    return this->thumbnailTimestamp != RecentFile::timestampLastModified(this->filepath);
}

int64_t RecentFile::timestampLastModified(const FilePath& fp)
{
    // Qt: QFileInfo(filepath).lastModified().toSecsSinceEpoch();
    try {
        const auto lastModifiedTime = filepathLastWriteTime(fp).time_since_epoch();
        return std::chrono::duration_cast<std::chrono::seconds>(lastModifiedTime).count();
    } catch (const std::exception& err) {
        qDebug() << fmt::format("Exception caught\n    Function {}\n    Filepath: {}\n    Error: {}",
                                Q_FUNC_INFO, fp.u8string(), err.what())
                    .c_str();
        return -1;
    }
}

bool operator==(const RecentFile& lhs, const RecentFile& rhs)
{
    if (lhs.filepath != rhs.filepath)
        return false;

    if (lhs.thumbnail.imageCacheKey >= 0 && rhs.thumbnail.imageCacheKey >= 0) {
        if (lhs.thumbnail.imageCacheKey != rhs.thumbnail.imageCacheKey)
            return false;
    }
    else if (lhs.thumbnail.imageData != rhs.thumbnail.imageData) {
        return false;
    }

    if (lhs.thumbnailTimestamp != rhs.thumbnailTimestamp)
        return false;

    return true;
}

template<> const char PropertyRecentFiles::TypeName[] = "Mayo::PropertyRecentFiles";

} // namespace Mayo
