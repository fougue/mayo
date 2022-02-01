/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_files.h"

#include "filepath_conv.h"
#include "theme.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"
#include "../gui/qtgui_utils.h"

#include <gsl/util>

#include <Graphic3d_GraphicDriver.hxx>

namespace Mayo {

namespace Internal {
// Defined in gui_create_virtual_window.cpp
Handle_Aspect_Window createGfxVirtualWindow(const Handle_Graphic3d_GraphicDriver&, int , int);
} // namespace Internal

static int64_t lastModifiedTimestamp(const FilePath& fp)
{
    // Qt: QFileInfo(filepath).lastModified().toSecsSinceEpoch();
    const auto lastModifiedTime = std::filesystem::last_write_time(fp).time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(lastModifiedTime).count();
}

bool RecentFile::recordThumbnail(GuiDocument* guiDoc, QSize size)
{
    if (!guiDoc)
        return false;

    if (!filepathEquivalent(this->filepath, guiDoc->document()->filePath()))
        return false;

    if (this->thumbnailTimestamp == lastModifiedTimestamp(this->filepath))
        return true;

    const GuiDocument::ViewTrihedronMode onEntryTrihedronMode = guiDoc->viewTrihedronMode();
    const bool onEntryOriginTrihedronVisible = guiDoc->isOriginTrihedronVisible();
    const QColor bkgColor = mayoTheme()->color(Theme::Color::Palette_Window);
    Handle_V3d_View view = guiDoc->graphicsScene()->createV3dView();
    view->ChangeRenderingParams().IsAntialiasingEnabled = true;
    view->ChangeRenderingParams().NbMsaaSamples = 4;
    view->SetBackgroundColor(QtGuiUtils::toPreferredColorSpace(bkgColor));

    auto _ = gsl::finally([=]{
        guiDoc->graphicsScene()->v3dViewer()->SetViewOff(view);
        guiDoc->setViewTrihedronMode(onEntryTrihedronMode);
        if (guiDoc->isOriginTrihedronVisible() != onEntryOriginTrihedronVisible)
            guiDoc->toggleOriginTrihedronVisibility();
    });

    guiDoc->graphicsScene()->clearSelection();
    guiDoc->setViewTrihedronMode(GuiDocument::ViewTrihedronMode::None);
    if (guiDoc->isOriginTrihedronVisible())
        guiDoc->toggleOriginTrihedronVisibility();

    auto wnd = Internal::createGfxVirtualWindow(view->Viewer()->Driver(), size.width(), size.height());
    view->SetWindow(wnd);

    GraphicsUtils::V3dView_fitAll(view);

    Image_PixMap pixmap;
    pixmap.SetTopDown(true);
    V3d_ImageDumpOptions dumpOptions;
    dumpOptions.BufferType = Graphic3d_BT_RGB;
    dumpOptions.Width = size.width();
    dumpOptions.Height = size.height();
    const bool ok = view->ToPixMap(pixmap, dumpOptions);
    if (!ok)
        return false;

    const QImage img(pixmap.Data(),
                     int(pixmap.Width()),
                     int(pixmap.Height()),
                     int(pixmap.SizeRowBytes()),
                     QImage::Format_RGB888);
    if (img.isNull())
        return false;

    this->thumbnail = QPixmap::fromImage(img);
    this->thumbnailTimestamp = lastModifiedTimestamp(this->filepath);

    return true;
}

bool RecentFile::isThumbnailOutOfSync() const
{
    return this->thumbnailTimestamp != lastModifiedTimestamp(this->filepath);
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
        if (stream.status() != QDataStream::Ok)
            break; // Stream extraction error, abort

        RecentFile recent;
        stream >> recent;
        if (!recent.filepath.empty() && recent.thumbnailTimestamp != 0)
            recentFiles.push_back(std::move(recent));
    }

    return stream;
}

template<> const char PropertyRecentFiles::TypeName[] = "Mayo::PropertyRecentFiles";

} // namespace Mayo
