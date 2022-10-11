/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_files.h"

#include "../base/meta_enum.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"
#include "../io_image/io_image.h"
#include "filepath_conv.h"
#include "qstring_conv.h"
#include "qtgui_utils.h"
#include "theme.h"

#include <fmt/format.h>
#include <QtCore/QtDebug>

namespace Mayo {

bool RecentFile::recordThumbnail(GuiDocument* guiDoc, QSize size)
{
    if (!guiDoc)
        return false;

    if (!filepathEquivalent(this->filepath, guiDoc->document()->filePath())) {
        qDebug() << fmt::format("Filepath mismatch with GUI document\n"
                                      "    Function: {}\n    Filepath: {}\n    Document: {}",
                                Q_FUNC_INFO, this->filepath.u8string(), guiDoc->document()->filePath().u8string())
                    .c_str();
        return false;
    }

    if (this->thumbnailTimestamp == RecentFile::timestampLastModified(this->filepath))
        return true;

    IO::ImageWriter::Parameters params;
    params.width = size.width();
    params.height = size.height();
    params.backgroundColor = QtGuiUtils::toPreferredColorSpace(mayoTheme()->color(Theme::Color::Palette_Window));
    Handle_Image_AlienPixMap pixmap = IO::ImageWriter::createImage(guiDoc, params);
    if (!pixmap) {
        qDebug() << "Empty pixmap returned by IO::ImageWriter::createImage()";
        return false;
    }

    GraphicsUtils::ImagePixmap_flipY(*pixmap);
    Image_PixMap::SwapRgbaBgra(*pixmap);
    this->thumbnail = QtGuiUtils::toQPixmap(*pixmap);
    this->thumbnailTimestamp = RecentFile::timestampLastModified(this->filepath);
    return true;
}

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
    return lhs.filepath == rhs.filepath
            && lhs.thumbnail.cacheKey() == rhs.thumbnail.cacheKey()
            && lhs.thumbnailTimestamp == rhs.thumbnailTimestamp;
}

QDataStream& operator<<(QDataStream& stream, const RecentFile& recentFile)
{
    stream << filepathTo<QString>(recentFile.filepath);
    stream << recentFile.thumbnail;
    stream << qint64(recentFile.thumbnailTimestamp);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, RecentFile& recentFile)
{
    QString strFilepath;
    stream >> strFilepath;
    recentFile.filepath = filepathFrom(strFilepath);
    stream >> recentFile.thumbnail;
    // Read thumbnail timestamp
    // Warning: qint64 and int64_t may not be the exact same type(eg __int64 and longlong with Windows/MSVC)
    qint64 timestamp;
    stream >> timestamp;
    recentFile.thumbnailTimestamp = timestamp;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const RecentFiles& recentFiles)
{
    stream << uint32_t(recentFiles.size());
    for (const RecentFile& recent : recentFiles)
        stream << recent;

    return stream;
}

QDataStream& operator>>(QDataStream& stream, RecentFiles& recentFiles)
{
    uint32_t count = 0;
    stream >> count;
    recentFiles.clear();
    for (uint32_t i = 0; i < count; ++i) {
        if (stream.status() != QDataStream::Ok) {
            qDebug() << fmt::format("QDataStream error\n    Function: {}\n    Status: {}",
                                    Q_FUNC_INFO, MetaEnum::name(stream.status()))
                        .c_str();
            break; // Stream extraction error, abort
        }

        RecentFile recent;
        stream >> recent;
        if (!recent.filepath.empty() && recent.thumbnailTimestamp != 0)
            recentFiles.push_back(std::move(recent));
    }

    return stream;
}

template<> const char PropertyRecentFiles::TypeName[] = "Mayo::PropertyRecentFiles";

} // namespace Mayo
