/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtCore/QElapsedTimer>
#include <QtCore/QFileInfo>
#include <QtWidgets/QWidget>
#include <QtWidgets/QFileIconProvider>
class QTreeWidget;
class QTreeWidgetItem;

namespace Mayo {

class WidgetFileSystem : public QWidget {
    Q_OBJECT
public:
    explicit WidgetFileSystem(QWidget* parent = nullptr);

    QFileInfo currentLocation() const;
    void setLocation(const QFileInfo& fiLoc);

signals:
    void locationActivated(const QFileInfo& loc);

private:
    void onTreeItemActivated(QTreeWidgetItem* item, int column);
    void onTreeItemClicked(QTreeWidgetItem* item, int column);
    void setCurrentFolder(const QDir& dir);
    void selectFileInCurrentFolder(const QString& fileName);

    QTreeWidget* m_treeWidget = nullptr;
    QFileInfo m_location;
    QFileIconProvider m_fileIconProvider;

    // Tap/activation timing(see onTreeItemClicked/onTreeItemActivated)
    QElapsedTimer m_clock;
    QTreeWidgetItem* m_lastTapItem = nullptr;
    qint64 m_lastTapTimeMs = -1;
    QTreeWidgetItem* m_lastActivatedItem = nullptr;
    qint64 m_lastActivatedTimeMs = -1000;
};

} // namespace Mayo
