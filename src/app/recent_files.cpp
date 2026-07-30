/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "recent_files.h"

#include "../base/filepath_conv.h"
#include "../base/meta_enum.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../qtcommon/filepath_conv.h"

#include <QtCore/QtDebug>
#include <QtCore/QDataStream>

#include <fmt/format.h>
#include <algorithm>

namespace Mayo {

namespace {

bool recordRecentFile(
        RecentFile* recentFile,
        GuiDocument* guiDoc,
        const PropertyRecentFiles::ThumbnailGeneratorFunction& fnThumbnailRecord,
        const QSize& thumbnailSize
    )
{
    if (!recentFile)
        return false;

    if (!guiDoc)
        return false;

    if (!fnThumbnailRecord)
        return false;

    if (!filepathEquivalent(recentFile->filepath, guiDoc->document()->filePath())) {
        qDebug() << fmt::format(
                        "Filepath mismatch with GUI document\n"
                        "    Function: {}\n    Filepath: {}\n    Document: {}",
                        Q_FUNC_INFO,
                        recentFile->filepath.u8string(),
                        guiDoc->document()->filePath().u8string()
                    ).c_str();
        return false;
    }

    if (recentFile->thumbnailTimestamp == RecentFile::timestampLastModified(recentFile->filepath))
        return true;

    recentFile->thumbnail = fnThumbnailRecord(guiDoc, thumbnailSize);
    recentFile->thumbnailTimestamp = RecentFile::timestampLastModified(recentFile->filepath);
    return true;
}

} // namespace

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

void RecentFileIO::read(QDataStream& stream, RecentFile* recentFile)
{
    QString strFilepath;
    stream >> strFilepath;
    if (stream.status() != QDataStream::Ok)
        return;

    recentFile->filepath = filepathFrom(strFilepath);
    stream >> recentFile->thumbnail.imageData;
    if (stream.status() != QDataStream::Ok)
        return;

    recentFile->thumbnail.imageCacheKey = -1;
    // Read thumbnail timestamp
    // Warning: qint64 and int64_t may not be the exact same type(eg __int64 and longlong with Windows/MSVC)
    qint64 timestamp;
    stream >> timestamp;
    if (stream.status() != QDataStream::Ok)
        return;

    recentFile->thumbnailTimestamp = timestamp;
}

void RecentFileIO::read(QDataStream& stream, RecentFiles* recentFiles)
{
    auto fnCheckStreamStatus = [](QDataStream::Status status) {
        if (status != QDataStream::Ok) {
            qDebug() << fmt::format(
                            "QDataStream error\n    Function: {}\n    Status: {}",
                            Q_FUNC_INFO, MetaEnum::name(status)
                            ).c_str();
            return false;
        }

        return true;
    };

    uint32_t count = 0;
    stream >> count;
    if (!fnCheckStreamStatus(stream.status()))
        return; // Stream extraction error, abort

    recentFiles->clear();
    for (uint32_t i = 0; i < count; ++i) {
        RecentFile recent;
        RecentFileIO::read(stream, &recent);
        if (!fnCheckStreamStatus(stream.status()))
            return; // Stream extraction error, abort

        if (!recent.filepath.empty() && recent.thumbnailTimestamp != 0)
            recentFiles->push_back(std::move(recent));
    }
}

void RecentFileIO::write(QDataStream& stream, const RecentFiles& recentFiles)
{
    stream << uint32_t(recentFiles.size());
    for (const RecentFile& rf : recentFiles) {
        stream << filepathTo<QString>(rf.filepath);
        stream << rf.thumbnail.imageData;
        stream << qint64(rf.thumbnailTimestamp);
    }
}

unsigned PropertyRecentFiles::countLimit() const
{
    return m_countLimit;
}

void PropertyRecentFiles::setCountLimit(unsigned limit)
{
    m_countLimit = limit;
}

QSize PropertyRecentFiles::thumbnailSize() const
{
    return m_thumbnailSize;
}

void PropertyRecentFiles::setThumbnailSize(int w, int h)
{
    m_thumbnailSize = QSize{w, h};
}

const PropertyRecentFiles::ThumbnailGeneratorFunction& PropertyRecentFiles::thumbnailGenerator() const
{
    return m_fnThumbnailGenerator;
}

void PropertyRecentFiles::setThumbnailGenerator(ThumbnailGeneratorFunction fn)
{
    m_fnThumbnailGenerator = std::move(fn);
}

void PropertyRecentFiles::prepend(const FilePath& fp)
{
    const RecentFile* ptrRecentFile = this->find(fp);
    RecentFiles newRecentFiles = this->value();
    if (ptrRecentFile) {
        RecentFile& firstRecentFile = newRecentFiles.front();
        RecentFile& recentFile = newRecentFiles.at(ptrRecentFile - &this->value().front());
        std::swap(firstRecentFile, recentFile);
    }
    else {
        RecentFile recentFile;
        recentFile.filepath = fp;
        newRecentFiles.insert(newRecentFiles.begin(), std::move(recentFile));
        while (newRecentFiles.size() > m_countLimit)
            newRecentFiles.pop_back();
    }

    this->setValue(newRecentFiles);
}

const RecentFile* PropertyRecentFiles::find(const FilePath& fp) const
{
    const RecentFiles& listRecentFile = this->value();
    auto itFound =
        std::find_if(
            listRecentFile.cbegin(),
            listRecentFile.cend(),
            [&](const RecentFile& recentFile) {
                return filepathEquivalent(fp, recentFile.filepath);
        });
    return itFound != listRecentFile.cend() ? &(*itFound) : nullptr;
}

void PropertyRecentFiles::record(GuiDocument* guiDoc)
{
    if (!guiDoc)
        return;

    if (guiDoc->document()->filePath().empty())
        return; // Anonymous document -> skip

    auto recentFile = this->find(guiDoc->document()->filePath());
    if (!recentFile) {
        qDebug() << fmt::format(
                        "RecentFile object is null\n"
                        "    Function: {}\n    Document: {}\n    RecentFilesCount: {}",
                        Q_FUNC_INFO,
                        guiDoc->document()->filePath().u8string(),
                        this->value().size()
                    ).c_str();
        return;
    }

    if (!recentFile->isThumbnailOutOfSync())
        return;

    RecentFile newRecentFile = *recentFile;
    const bool okRecord = recordRecentFile(&newRecentFile, guiDoc, m_fnThumbnailGenerator, m_thumbnailSize);
    if (!okRecord)
        return;

    const RecentFiles& listRecentFile = this->value();
    RecentFiles newListRecentFile = listRecentFile;
    const auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
    newListRecentFile.at(indexRecentFile) = newRecentFile;
    this->setValue(newListRecentFile);
}

void PropertyRecentFiles::record(GuiApplication* guiApp)
{
    if (!guiApp)
        return;

    const RecentFiles& listRecentFile = this->value();
    RecentFiles newListRecentFile = listRecentFile;
    for (GuiDocument* guiDoc : guiApp->guiDocuments()) {
        auto recentFile = this->find(guiDoc->document()->filePath());
        if (!recentFile || !recentFile->isThumbnailOutOfSync())
            continue; // Skip

        RecentFile newRecentFile = *recentFile;
        if (recordRecentFile(&newRecentFile, guiDoc, m_fnThumbnailGenerator, m_thumbnailSize)) {
            auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
            newListRecentFile.at(indexRecentFile) = newRecentFile;
        }
    }

    this->setValue(newListRecentFile);
}

template<> const char GenericProperty<RecentFiles>::TypeName[] = "Mayo::PropertyRecentFiles";

} // namespace Mayo
