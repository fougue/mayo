/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_home_files.h"

#include "../base/application.h"
#include "../base/settings.h"
#include "../base/string_utils.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "app_module.h"
#include "theme.h"

#include <QtCore/QtDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QPixmapCache>
#include <QtWidgets/QFileIconProvider>
#include <QtWidgets/QVBoxLayout>
#include <algorithm>

namespace Mayo {

namespace {

struct HomeFileItem : public ListHelper::ModelItem {
    enum class Type { None, New, Open, RecentFile };
    Type type = Type::None;
    QString filepath;
};

class HomeFilesModel : public ListHelper::Model {
public:
    HomeFilesModel(QObject* parent)
        : ListHelper::Model(parent)
    {
        auto storage = std::make_unique<ListHelper::DefaultModelStorage<HomeFileItem>>();
        m_storage = storage.get();

        { // "New Document" item
            HomeFileItem item;
            item.name = WidgetHomeFiles::tr("New Document");
            item.description = WidgetHomeFiles::tr(
                        "\n\nCreate and add an empty document where you can import files");
            item.imageUrl = ImageId_NewDocument;
            item.type = HomeFileItem::Type::New;
            item.textWrapMode = QTextOption::WordWrap;
            storage->m_items.push_back(std::move(item));
        }

        { // "Open Documents" item
            HomeFileItem item;
            item.name = WidgetHomeFiles::tr("Open Document(s)");
            item.description = WidgetHomeFiles::tr(
                        "\n\nSelect files to load and open as distinct documents");
            item.imageUrl = ImageId_OpenDocuments;
            item.type = HomeFileItem::Type::Open;
            item.textWrapMode = QTextOption::WordWrap;
            storage->m_items.push_back(std::move(item));
        }

        this->reloadRecentFiles();
        this->setStorage(std::move(storage));
    }

    QPixmap findPixmap(const QString& url) const override {
        QPixmap pixmap;
        auto fnPixmap = [](const QIcon& icon, int iconSize, int pixmapSize) {
            return icon
                    .pixmap(iconSize, iconSize)
                    .scaled(pixmapSize, pixmapSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        };
        if (url == ImageId_NewDocument) {
            pixmap = fnPixmap(mayoTheme()->icon(Theme::Icon::AddFile), 128, 96);
        }
        else if (url == ImageId_OpenDocuments) {
            pixmap = fnPixmap(mayoTheme()->icon(Theme::Icon::OpenFiles), 128, 96);
        }
        else {
            auto appModule = AppModule::get(Application::instance());
            const RecentFile* recentFile = appModule ? appModule->findRecentFile(url) : nullptr;
            pixmap = recentFile ? recentFile->thumbnail : QPixmap();
            if (pixmap.isNull()) {
                const QIcon icon = m_fileIconProvider.icon(QFileInfo(url));
                pixmap = fnPixmap(icon, 64, 64);
            }
        }

        QPixmapCache::insert(url, pixmap);
        return pixmap;
    }

    void reload() {
        this->beginResetModel();
        this->reloadRecentFiles();
        this->endResetModel();
    }

private:
    void reloadRecentFiles() {
        auto app = Application::instance();
        auto appModule = AppModule::get(app);
        const RecentFiles& listRecentFile = appModule->recentFiles.value();
        if (m_cacheRecentFiles == listRecentFile)
            return;

        m_storage->m_items.erase(m_storage->m_items.begin() + 2, m_storage->m_items.end());
        auto fnToString = [=](const QDateTime& dateTime) {
            const QString strTime = dateTime.time().toString("HH:mm");
            const QDate date = dateTime.date();
            const QDate currentDate = QDate::currentDate();
            const int diffDays = date.daysTo(currentDate);
            if (diffDays == 0) {
                return WidgetHomeFiles::tr("today %1").arg(strTime);
            }
            else if (diffDays == 1) {
                return WidgetHomeFiles::tr("yersterday %1").arg(strTime);
            }
            else if (date.year() == currentDate.year() && date.weekNumber() == currentDate.weekNumber()) {
                const QString strDayName = date.toString("dddd");
                return WidgetHomeFiles::tr("%1 %2").arg(strDayName, strTime);
            }
            else if (diffDays < 5) {
                return WidgetHomeFiles::tr("%1 days ago %2").arg(diffDays).arg(strTime);
            }
            else {
                const QString strDate = app->settings()->locale().toString(date, QLocale::ShortFormat);
                return WidgetHomeFiles::tr("%1 %2").arg(strDate, strTime);
            }
        };
        for (const RecentFile& recentFile : listRecentFile) {
            HomeFileItem item;
            const QFileInfo fi(recentFile.filepath);
            item.name = fi.fileName();
            item.type = HomeFileItem::Type::RecentFile;
            item.description =
                    WidgetHomeFiles::tr(
                        "%1\n\n"
                        "Size: %2\n\n"
                        "Created: %3\n"
                        "Modified: %4\n"
                        "Read: %5\n")
                    .arg(QDir::toNativeSeparators(fi.absolutePath()))
                    .arg(StringUtils::bytesText(fi.size(), app->settings()->locale()))
                    .arg(fnToString(fi.birthTime()))
                    .arg(fnToString(fi.lastModified()))
                    .arg(fnToString(fi.lastRead()))
                    ;
            item.textWrapMode = QTextOption::WrapAtWordBoundaryOrAnywhere;
            item.imageUrl = recentFile.filepath;
            item.filepath = recentFile.filepath;
            m_storage->m_items.push_back(std::move(item));
        }

        m_cacheRecentFiles = listRecentFile;
    }

    static constexpr const char ImageId_NewDocument[] = "NewDocument_beae5f60-78a5-4b4e-8875-2dcebdbb4c58";
    static constexpr const char ImageId_OpenDocuments[] = "OpenDocuments_945b1913-59fb-4150-9000-f66332f850fe";

    QFileIconProvider m_fileIconProvider;
    RecentFiles m_cacheRecentFiles;
    ListHelper::DefaultModelStorage<HomeFileItem>* m_storage = nullptr;
};

class HomeFilesDelegate : public ListHelper::ItemDelegate {
public:
    HomeFilesDelegate(WidgetHomeFiles* widget)
        : ListHelper::ItemDelegate(widget),
          m_widget(widget)
    {}

protected:
    void clickAction(const ListHelper::ModelItem* item) const override {
        auto fileItem = static_cast<const HomeFileItem*>(item);
        if (!fileItem)
            return;

        if (fileItem->type == HomeFileItem::Type::New)
            emit m_widget->newDocumentRequested();
        else if (fileItem->type == HomeFileItem::Type::Open)
            emit m_widget->openDocumentsRequested();
        else if (fileItem->type == HomeFileItem::Type::RecentFile)
            emit m_widget->recentFileOpenRequested(fileItem->filepath);
    }

private:
    WidgetHomeFiles* m_widget = nullptr;
};

} // namespace

WidgetHomeFiles::WidgetHomeFiles(QWidget* parent)
    : QWidget(parent)
{
    auto app = Application::instance();
    auto appModule = AppModule::get(app);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto model = new HomeFilesModel(this);
    m_gridModel.setSourceModel(model);

    m_gridView = new GridHelper::View(this);
    m_gridView->setModel(&m_gridModel);
    layout->addWidget(m_gridView);

    m_gridDelegate = new HomeFilesDelegate(this);
    m_gridDelegate->setItemSize(m_gridView->itemSize());
    m_gridDelegate->setItemPixmapSize(appModule->recentFileThumbnailSize());
    m_gridView->setItemDelegate(m_gridDelegate);

    QObject::connect(app->settings(), &Settings::changed, this, [=](const Property* setting) {
        if (setting == &appModule->recentFiles)
            model->reload();
    });
}

void WidgetHomeFiles::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const int gridColumnCount = this->width() / m_gridView->itemSize().width();
    m_gridModel.setColumnCount(std::max(1, gridColumnCount));
}

void WidgetHomeFiles::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    auto homeFilesModel = static_cast<HomeFilesModel*>(m_gridModel.sourceModel());
    homeFilesModel->reload();
}

} // namespace Mayo
