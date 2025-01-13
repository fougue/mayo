/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"
#include <QtWidgets/QDialog>
#include <TDocStd_Document.hxx>
class QTreeWidgetItem;

namespace Mayo {

class DialogInspectXde : public QDialog {
    Q_OBJECT
public:
    DialogInspectXde(QWidget* parent = nullptr);
    ~DialogInspectXde();

    void load(const OccHandle<TDocStd_Document>& doc);

private:
    void onLabelTreeWidgetItemClicked(QTreeWidgetItem* item, int column);

    class Ui_DialogInspectXde* m_ui = nullptr;
    OccHandle<TDocStd_Document> m_doc;
};

} // namespace Mayo
