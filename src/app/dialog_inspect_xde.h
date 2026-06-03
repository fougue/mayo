/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
    explicit DialogInspectXde(QWidget* parent = nullptr);
    ~DialogInspectXde();

    void load(const OccHandle<TDocStd_Document>& doc);

private:
    void onLabelTreeWidgetItemClicked(QTreeWidgetItem* item, int column);

    class Ui_DialogInspectXde* m_ui = nullptr;
    OccHandle<TDocStd_Document> m_doc;
};

} // namespace Mayo
