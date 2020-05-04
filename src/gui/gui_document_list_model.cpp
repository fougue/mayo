/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document_list_model.h"

#include "../base/document.h"
#include "gui_application.h"
#include "gui_document.h"

namespace Mayo {

GuiDocumentListModel::GuiDocumentListModel(GuiApplication* app)
    : QStringListModel(app)
{
    for (const GuiDocument* doc : app->guiDocuments())
        this->appendGuiDocument(doc);

    QObject::connect(
                app, &GuiApplication::guiDocumentAdded,
                this, &GuiDocumentListModel::appendGuiDocument);
    QObject::connect(
                app, &GuiApplication::guiDocumentErased,
                this, &GuiDocumentListModel::removeGuiDocument);
}

QVariant GuiDocumentListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && role == Qt::ToolTipRole)
        return m_vecGuiDocument.at(index.row())->document()->filePath();
    else
        return QStringListModel::data(index, role);
}

void GuiDocumentListModel::appendGuiDocument(const GuiDocument* guiDoc)
{
    const int rowId = this->rowCount();
    this->insertRow(rowId);
    const QModelIndex indexRow = this->index(rowId);
    this->setData(indexRow, guiDoc->document()->name());
    m_vecGuiDocument.emplace_back(guiDoc);
}

void GuiDocumentListModel::removeGuiDocument(const GuiDocument* guiDoc)
{
    auto itFound = std::find(m_vecGuiDocument.begin(), m_vecGuiDocument.end(), guiDoc);
    if (itFound != m_vecGuiDocument.end()) {
        this->removeRow(itFound - m_vecGuiDocument.begin());
        m_vecGuiDocument.erase(itFound);
    }
}

} // namespace Mayo
