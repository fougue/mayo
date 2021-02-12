/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_files.h"

#include "occt_window.h"
#include "theme.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"
#include "../gui/qtgui_utils.h"

#include <gsl/gsl_util>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtWidgets/QWidget>

namespace Mayo {

bool RecentFile::recordThumbnail(GuiDocument* guiDoc, QSize size)
{
    if (!guiDoc)
        return false;

    const QFileInfo fileInfo(this->filepath);
    if (fileInfo != QFileInfo(guiDoc->document()->filePath()))
        return false;

    const int64_t lastModifiedTimestamp = fileInfo.lastModified().toSecsSinceEpoch();
    if (this->thumbnailTimestamp != lastModifiedTimestamp) {
        const GuiDocument::ViewTrihedronMode onEntryTrihedronMode = guiDoc->viewTrihedronMode();
        const bool onEntryOriginTrihedronVisible = guiDoc->isOriginTrihedronVisible();
        const QColor backgroundColor = mayoTheme()->color(Theme::Color::Palette_Window);
        Handle_V3d_View view = guiDoc->graphicsScene()->createV3dView();
        view->ChangeRenderingParams().IsAntialiasingEnabled = true;
        view->ChangeRenderingParams().NbMsaaSamples = 4;
        view->SetBackgroundColor(QtGuiUtils::toColor<Quantity_Color>(backgroundColor));

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

        QWidget widgetView; // TODO Use a pure offscreen window instead
        Handle_Aspect_Window hWnd = new OcctWindow(&widgetView);
        view->SetWindow(hWnd);
        view->MustBeResized();
        GraphicsUtils::V3dView_fitAll(view);
        view->Redraw();

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
        this->thumbnailTimestamp = lastModifiedTimestamp;
    }

    return true;
}

bool RecentFile::isThumbnailOutOfSync() const
{
    const QFileInfo fileInfo(this->filepath);
    const int64_t lastModifiedTimestamp = fileInfo.lastModified().toSecsSinceEpoch();
    return this->thumbnailTimestamp != lastModifiedTimestamp;
}

bool operator==(const RecentFile& lhs, const RecentFile& rhs)
{
    return lhs.filepath == rhs.filepath
            && lhs.thumbnail.cacheKey() == rhs.thumbnail.cacheKey()
            && lhs.thumbnailTimestamp == rhs.thumbnailTimestamp;
}

QDataStream& operator<<(QDataStream& stream, const RecentFile& recentFile)
{
    stream << recentFile.filepath;
    stream << recentFile.thumbnail;
    stream << qint64(recentFile.thumbnailTimestamp);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, RecentFile& recentFile)
{
    stream >> recentFile.filepath;
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
        RecentFile recent;
        stream >> recent;
        recentFiles.push_back(std::move(recent));
    }

    return stream;
}

template<> const char PropertyRecentFiles::TypeName[] = "Mayo::PropertyRecentFiles";

} // namespace Mayo
