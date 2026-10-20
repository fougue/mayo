/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath.h"

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

    // Display the folder containing 'filePath' and highlight that file
    // Unlike setLocation() this performs no synchronous filesystem query: the folder content is
    // listed asynchronously and the file is selected once the listing is done
    void setDocumentLocation(const FilePath& filePath);

signals:
    void locationActivated(const QFileInfo& loc);

private:
    void onTreeItemActivated(QTreeWidgetItem* item, int column);
    void onTreeItemClicked(QTreeWidgetItem* item, int column);
    void onTreeItemEntered(QTreeWidgetItem* item, int column);
    void requestFolder(const QString& folderPath, const QString& fileNameToSelect);
    void populateFolder(quint64 generation, const QFileInfoList& listEntryFileInfo);
    void selectFileInCurrentFolder(const QString& fileName);

    QTreeWidget* m_treeWidget = nullptr;
    QFileInfo m_location;
    QFileIconProvider m_fileIconProvider;

    // Asynchronous folder listing state
    quint64 m_scanGeneration = 0;
    bool m_scanPending = false;
    bool m_folderIsSet = false;
    QString m_currentFolderPath;
    QString m_fileToSelect;

    // Tap/activation timing(see onTreeItemClicked/onTreeItemActivated)
    QElapsedTimer m_clock;
    QTreeWidgetItem* m_lastTapItem = nullptr;
    qint64 m_lastTapTimeMs = -1;
    QTreeWidgetItem* m_lastActivatedItem = nullptr;
    qint64 m_lastActivatedTimeMs = -1000;
};

} // namespace Mayo
