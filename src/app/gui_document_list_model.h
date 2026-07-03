/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include <QtCore/QAbstractListModel>
#include <vector>

namespace Mayo {

class GuiApplication;
class GuiDocument;

// Provides a Qt item model of the documents owned by a GuiApplication object
// Contents is automatically updated in reaction of Application and GuiApplication signals
class GuiDocumentListModel : public QAbstractListModel {
public:
    GuiDocumentListModel(const GuiApplication* guiApp, QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    void appendGuiDocument(const GuiDocument* guiDoc);
    void removeGuiDocument(const GuiDocument* guiDoc);
    void onDocumentNameChanged(const DocumentPtr& doc, const std::string& name);

    std::vector<const GuiDocument*> m_vecGuiDocument;
};

} // namespace Mayo
