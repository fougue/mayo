/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_files.h"

#include "filepath_conv.h"
#include "occt_window.h"
#include "theme.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"
#include "../gui/qtgui_utils.h"

#include <gsl/util>
//#include <QtGui/QOffscreenSurface>
#include <QtGui/QWindow>
#include <Aspect_NeutralWindow.hxx>

namespace Mayo {

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

    if (this->thumbnailTimestamp != lastModifiedTimestamp(this->filepath)) {
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

        QWindow window; // TODO Use a pure offscreen window instead
        window.setBaseSize(size);
        window.create();
        Handle_Aspect_NeutralWindow hWnd = new Aspect_NeutralWindow;
        hWnd->SetSize(size.width(), size.height());
        hWnd->SetNativeHandle(Aspect_Drawable(window.winId()));
        view->SetWindow(hWnd);

        GraphicsUtils::V3dView_fitAll(view);

        Image_PixMap pixmap;
        pixmap.SetTopDown(true);
        V3d_ImageDumpOptions dumpOptions;
        dumpOptions.BufferType = Graphic3d_BT_RGB;
        dumpOptions.Width = size.width();
        dumpOptions.Height = size.height();
        bool ok = view->ToPixMap(pixmap, dumpOptions);
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
    }

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
