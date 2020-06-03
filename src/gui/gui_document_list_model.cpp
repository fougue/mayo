/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document_list_model.h"

#include "../base/application.h"
#include "../base/document.h"
#include "gui_application.h"
#include "gui_document.h"

namespace Mayo {

GuiDocumentListModel::GuiDocumentListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    auto app = GuiApplication::instance();
    for (const GuiDocument* doc : app->guiDocuments())
        this->appendGuiDocument(doc);

    QObject::connect(
                app, &GuiApplication::guiDocumentAdded,
                this, &GuiDocumentListModel::appendGuiDocument);
    QObject::connect(
                app, &GuiApplication::guiDocumentErased,
                this, &GuiDocumentListModel::removeGuiDocument);
    QObject::connect(
                Application::instance().get(), &Application::documentNameChanged,
                this, &GuiDocumentListModel::onDocumentNameChanged);
}

QVariant GuiDocumentListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_vecGuiDocument.size())
        return QVariant();

    const DocumentPtr& doc = m_vecGuiDocument.at(index.row())->document();
    switch (role) {
    case Qt::ToolTipRole:
        return doc->filePath();
    case Qt::DisplayRole:
    case Qt::EditRole:
        return doc->name();
    }

    return QVariant();
}

int GuiDocumentListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_vecGuiDocument.size();
}

void GuiDocumentListModel::appendGuiDocument(const GuiDocument* guiDoc)
{
    const int row = this->rowCount();
    this->beginInsertRows(QModelIndex(), row, row);
    m_vecGuiDocument.emplace_back(guiDoc);
    this->endInsertRows();
}

void GuiDocumentListModel::removeGuiDocument(const GuiDocument* guiDoc)
{
    auto itFound = std::find(m_vecGuiDocument.begin(), m_vecGuiDocument.end(), guiDoc);
    if (itFound != m_vecGuiDocument.end()) {
        const int row = itFound - m_vecGuiDocument.begin();
        this->beginRemoveRows(QModelIndex(), row, row);
        m_vecGuiDocument.erase(itFound);
        this->endRemoveRows();
    }
}

void GuiDocumentListModel::onDocumentNameChanged(const DocumentPtr& doc, const QString& /*name*/)
{
    auto itFound = std::find_if(
                m_vecGuiDocument.cbegin(),
                m_vecGuiDocument.cend(),
                [&](const GuiDocument* guiDoc) { return guiDoc->document() == doc; });
    if (itFound != m_vecGuiDocument.cend()) {
        const int row = itFound - m_vecGuiDocument.begin();
        const QModelIndex itemIndex = this->index(row);
        emit this->dataChanged(itemIndex, itemIndex, { Qt::DisplayRole, Qt::EditRole });
    }
}

} // namespace Mayo
