/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "widget_file_system.h"
#include "qstring_utils.h"
#include "qtwidgets_utils.h"

#include "../qtcommon/filepath_conv.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QMetaObject>
#include <QtCore/QPointer>
#include <QtCore/QThreadPool>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QScroller>
#include <QtWidgets/QTreeWidget>

namespace Mayo {

namespace Internal {

static QString absolutePath(const QFileInfo& fi)
{
    return fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
}

} // namespace Internal

WidgetFileSystem::WidgetFileSystem(QWidget* parent)
    : QWidget(parent),
      m_treeWidget(new QTreeWidget(this))
{
    auto layout = new QVBoxLayout;
    layout->addWidget(m_treeWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setIndentation(0);
    // Mouse tracking is required so itemEntered() gets emitted: this is used to build the item
    // tooltip(sizes, last modification date, ...) on demand instead of upfront for every entry
    m_treeWidget->setMouseTracking(true);

    // Enable flick scrolling with a finger on touch screens
    m_treeWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_treeWidget->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(m_treeWidget->viewport(), QScroller::TouchGesture);

    QObject::connect(
        m_treeWidget, &QTreeWidget::itemActivated,
        this, &WidgetFileSystem::onTreeItemActivated
    );
    // Touch sessions: a single tap must only highlight/select the entry, opening a file requires
    // an explicit double-tap(see onTreeItemClicked). Otherwise casual taps while flick-scrolling
    // would open files
    m_clock.start();
    QObject::connect(
        m_treeWidget, &QTreeWidget::itemClicked,
        this, &WidgetFileSystem::onTreeItemClicked
    );
    QObject::connect(
        m_treeWidget, &QTreeWidget::itemEntered,
        this, &WidgetFileSystem::onTreeItemEntered
    );
}

QFileInfo WidgetFileSystem::currentLocation() const
{
    return m_location;
}

void WidgetFileSystem::setLocation(const QFileInfo& fiLoc)
{
    // This entry point is used for in-app navigation: the caller usually already queried the
    // QFileInfo metadata(eg to know whether this is a directory) so these queries are served from
    // cache and do not induce extra filesystem round-trips
    const QString folderPath = fiLoc.isDir() ? fiLoc.absoluteFilePath() : fiLoc.absolutePath();
    this->requestFolder(folderPath, fiLoc.fileName());
    m_location = fiLoc;
}

void WidgetFileSystem::setDocumentLocation(const FilePath& filePath)
{
    const QString folderPath = filepathTo<QString>(filePath.parent_path());
    const QString fileName = filepathTo<QString>(filePath.filename());
    this->requestFolder(folderPath, fileName);
    // NOTE: constructing a QFileInfo does not query the filesystem
    m_location = QFileInfo(QDir(folderPath), fileName);
}

void WidgetFileSystem::onTreeItemActivated(QTreeWidgetItem* item, int column)
{
    if (item != nullptr && column == 0) {
        // Guard against duplicate activation of the same entry within a short time frame: a touch
        // double-tap may both synthesize a native double-click activation(and thus this function
        // being called) and trigger the manual double-tap detection(see onTreeItemClicked)
        const qint64 nowMs = m_clock.elapsed();
        const bool isDuplicateActivation =
            item == m_lastActivatedItem
            && m_lastActivatedTimeMs >= 0
            && (nowMs - m_lastActivatedTimeMs) <= QApplication::doubleClickInterval()
        ;
        m_lastActivatedItem = item;
        m_lastActivatedTimeMs = nowMs;
        if (isDuplicateActivation)
            return;

        if (item->text(0) == QLatin1String("..")) {
            QDir dir = Internal::absolutePath(m_location);
            dir.cdUp();
            this->requestFolder(dir.absolutePath(), QString());
            m_location = QFileInfo(dir.absolutePath());
        }
        else {
            const QDir dir(Internal::absolutePath(m_location));
            const QFileInfo fi(dir, item->text(0));
            if (fi.isDir()) {
                this->setLocation(fi);
            }
            else {
                emit this->locationActivated(fi);
                m_location = fi;
            }
        }
    }
}

void WidgetFileSystem::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
    if (!item || column != 0)
        return;

    // Desktop sessions: single click only highlights/selects the item, itemActivated() handles
    // activation on double-click/Enter key
    if (!QtWidgetsUtils::isTouchSessionActive())
        return;

    const qint64 nowMs = m_clock.elapsed();
    const bool isDoubleTap =
        item == m_lastTapItem
        && m_lastTapTimeMs >= 0
        && (nowMs - m_lastTapTimeMs) <= QApplication::doubleClickInterval()
    ;
    m_lastTapItem = item;
    m_lastTapTimeMs = nowMs;
    if (isDoubleTap)
        this->onTreeItemActivated(item, column);
}

void WidgetFileSystem::onTreeItemEntered(QTreeWidgetItem* item, int column)
{
    if (!item || column != 0)
        return;

    const QVariant varFileInfo = item->data(0, Qt::UserRole);
    if (!varFileInfo.canConvert<QFileInfo>())
        return;

    // Resolve size and last modification date on demand: these imply filesystem queries that may
    // be slow on network locations, so they must not run for every entry when populating the tree
    const QFileInfo fi = varFileInfo.value<QFileInfo>();
    const QString itemTooltip =
            tr("%1\nSize: %2\nLast modified: %3")
            .arg(QDir::toNativeSeparators(fi.absoluteFilePath()))
            .arg(QStringUtils::bytesText(fi.size()))
            .arg(QLocale::system().toString(fi.lastModified(), QLocale::LongFormat))
        ;
    item->setToolTip(0, itemTooltip);
}

void WidgetFileSystem::requestFolder(const QString& folderPath, const QString& fileNameToSelect)
{
    m_fileToSelect = fileNameToSelect;
    const bool sameFolder = m_folderIsSet && (folderPath == m_currentFolderPath);
    if (sameFolder) {
        // Nothing to rescan: either select right now, or let the in-flight listing do it when done
        if (!m_scanPending)
            this->selectFileInCurrentFolder(m_fileToSelect);
        return;
    }

    m_folderIsSet = true;
    m_currentFolderPath = folderPath;
    m_scanPending = true;
    const quint64 generation = ++m_scanGeneration;

    m_treeWidget->clear();
    m_treeWidget->headerItem()->setText(0, QDir(folderPath).dirName());

    QPointer<WidgetFileSystem> self(this);
    QThreadPool::globalInstance()->start(
        [self, folderPath, generation]{
            // Directory scan and per-entry metadata queries are performed on a worker thread: on a
            // slow(eg network) location these imply many filesystem round-trips and would otherwise
            // freeze the UI
            const QFileInfoList listEntryFileInfo =
                QDir(folderPath).entryInfoList(
                        QDir::Files | QDir::AllDirs | QDir::NoDot,
                        QDir::DirsFirst
                );
            if (!self)
                return;

            QMetaObject::invokeMethod(
                self.data(),
                [self, generation, listEntryFileInfo]{
                    if (self)
                        self->populateFolder(generation, listEntryFileInfo);
                },
                Qt::QueuedConnection
            );
        }
    );
}

void WidgetFileSystem::populateFolder(quint64 generation, const QFileInfoList& listEntryFileInfo)
{
    if (generation != m_scanGeneration)
        return; // Discard the results of an outdated scan

    m_scanPending = false;
    QList<QTreeWidgetItem*> listItem;
    for (const QFileInfo& fi : listEntryFileInfo) {
        auto item = new QTreeWidgetItem;
        if (fi.fileName() != QLatin1String(".")) {
            item->setText(0, fi.fileName());
            // Use generic icons: per-file icons rely on MIME/shell lookups that may access the
            // file and are therefore very slow on network locations
            const QFileIconProvider::IconType iconType =
                fi.isDir() ? QFileIconProvider::Folder : QFileIconProvider::File;
            item->setIcon(0, m_fileIconProvider.icon(iconType));
            // Only the file path is cheap to compute here. Size/last modification are resolved
            // on demand in onTreeItemEntered() to avoid one metadata query per entry
            item->setToolTip(0, QDir::toNativeSeparators(fi.absoluteFilePath()));
            item->setData(0, Qt::UserRole, QVariant::fromValue(fi));
        }

        listItem.push_back(item);
    }

    m_treeWidget->addTopLevelItems(listItem);
    this->selectFileInCurrentFolder(m_fileToSelect);
}

void WidgetFileSystem::selectFileInCurrentFolder(const QString& fileName)
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        if (m_treeWidget->topLevelItem(i)->text(0) == fileName) {
            m_treeWidget->clearSelection();
            m_treeWidget->topLevelItem(i)->setSelected(true);
            return;
        }
    }
}

} // namespace Mayo
