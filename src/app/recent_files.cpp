/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_files.h"

#include "../base/meta_enum.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"
#include "../gui/qtgui_utils.h"
#include "../io_image/io_image.h"
#include "filepath_conv.h"
#include "qstring_conv.h"
#include "theme.h"

#include <QtCore/QtDebug>

namespace Mayo {

bool RecentFile::recordThumbnail(GuiDocument* guiDoc, QSize size)
{
    if (!guiDoc)
        return false;

    if (!filepathEquivalent(this->filepath, guiDoc->document()->filePath())) {
        qDebug().noquote() << QString("Filepath mismatch with GUI document\n"
                                      "    Function: %1\n    Filepath: %2\n    Document: %2")
                              .arg(Q_FUNC_INFO)
                              .arg(filepathTo<QString>(this->filepath))
                              .arg(filepathTo<QString>(guiDoc->document()->filePath()));
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
        qDebug().noquote() << "Empty pixmap returned by IO::ImageWriter::createImage()";
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
        const auto lastModifiedTime = std::filesystem::last_write_time(fp).time_since_epoch();
        return std::chrono::duration_cast<std::chrono::seconds>(lastModifiedTime).count();
    } catch (const std::exception& err) {
        qDebug().noquote() << QString("Exception caught\n    Function %1\n    Filepath: %2\n    Error: %3")
                              .arg(Q_FUNC_INFO, filepathTo<QString>(fp), err.what());
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
    stream >> reinterpret_cast<qint64&>(recentFile.thumbnailTimestamp);
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
            qDebug().noquote() << QString("QDataStream error\n    Function: %1\n    Status: %2")
                                  .arg(Q_FUNC_INFO, to_QString(MetaEnum::name(stream.status())));
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
