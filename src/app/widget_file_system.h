/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QFileInfo>
#include <QtWidgets/QWidget>
#include <QtWidgets/QFileIconProvider>
class QTreeWidget;
class QTreeWidgetItem;

namespace Mayo {

class WidgetFileSystem : public QWidget {
    Q_OBJECT
public:
    WidgetFileSystem(QWidget* parent = nullptr);

    QFileInfo currentLocation() const;
    void setLocation(const QFileInfo& fiLoc);

signals:
    void locationActivated(const QFileInfo& loc);

private:
    void onTreeItemActivated(QTreeWidgetItem* item, int column);
    void setCurrentFolder(const QDir& dir);
    void selectFileInCurrentFolder(const QString& fileName);

    QTreeWidget* m_treeWidget = nullptr;
    QFileInfo m_location;
    QFileIconProvider m_fileIconProvider;
};

} // namespace Mayo
