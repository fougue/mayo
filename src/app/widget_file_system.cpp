/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "widget_file_system.h"
#include "qstring_utils.h"
#include "qtwidgets_utils.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
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
}

QFileInfo WidgetFileSystem::currentLocation() const
{
    return m_location;
}

void WidgetFileSystem::setLocation(const QFileInfo& fiLoc)
{
    const QString pathCurrLocation = Internal::absolutePath(m_location);
    const QString pathLoc = Internal::absolutePath(fiLoc);
    if (pathCurrLocation == pathLoc && fiLoc.isFile()) {
        this->selectFileInCurrentFolder(fiLoc.fileName());
    }
    else {
        this->setCurrentFolder(pathLoc);
        this->selectFileInCurrentFolder(fiLoc.fileName());
    }

    m_location = fiLoc;
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
            this->setCurrentFolder(dir);
            m_location = dir.absolutePath();
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

void WidgetFileSystem::setCurrentFolder(const QDir& dir)
{
    m_treeWidget->clear();
    QList<QTreeWidgetItem*> listItem;
    if (dir.exists()) {
        const QFileInfoList listEntryFileInfo =
            dir.entryInfoList(
                    QDir::Files | QDir::AllDirs | QDir::NoDot,
                    QDir::DirsFirst
            );
        for (const QFileInfo& fi : listEntryFileInfo) {
            auto item = new QTreeWidgetItem;
            if (fi.fileName() != QLatin1String(".")) {
                item->setText(0, fi.fileName());
                item->setIcon(0, m_fileIconProvider.icon(fi));
                const QString itemTooltip =
                        tr("%1\nSize: %2\nLast modified: %3")
                        .arg(QDir::toNativeSeparators(fi.absoluteFilePath()))
                        .arg(QStringUtils::bytesText(fi.size()))
                        .arg(QLocale::system().toString(fi.lastModified(), QLocale::LongFormat))
                    ;
                item->setToolTip(0, itemTooltip);
            }

            listItem.push_back(item);
        }
    }

    m_treeWidget->addTopLevelItems(listItem);
    m_treeWidget->headerItem()->setText(0, dir.dirName());
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
