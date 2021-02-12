/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "grid_helper.h"
#include "list_helper.h"

namespace Mayo {

class WidgetHomeFiles : public QWidget {
    Q_OBJECT
public:
    WidgetHomeFiles(QWidget* parent = nullptr);

signals:
    void newDocumentRequested();
    void openDocumentsRequested();
    void recentFileOpenRequested(const QString& filepath);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    GridHelper::View* m_gridView;
    GridHelper::ProxyModel m_gridModel;
    ListHelper::ItemDelegate* m_gridDelegate;
};

} // namespace Mayo
