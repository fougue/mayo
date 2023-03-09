/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
