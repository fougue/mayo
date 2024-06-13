/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_file_system.h"
#include "qstring_utils.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtWidgets/QBoxLayout>
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

    QObject::connect(
        m_treeWidget, &QTreeWidget::itemActivated,
        this, &WidgetFileSystem::onTreeItemActivated
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
