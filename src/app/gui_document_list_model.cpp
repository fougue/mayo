/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document_list_model.h"

#include "../base/application.h"
#include "../base/document.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"

namespace Mayo {

GuiDocumentListModel::GuiDocumentListModel(const GuiApplication* guiApp, QObject* parent)
    : QAbstractListModel(parent)
{
    for (const GuiDocument* doc : guiApp->guiDocuments())
        this->appendGuiDocument(doc);

    auto app = guiApp->application();
    app->signalDocumentNameChanged.connectSlot(&GuiDocumentListModel::onDocumentNameChanged, this);
    guiApp->signalGuiDocumentAdded.connectSlot(&GuiDocumentListModel::appendGuiDocument, this);
    guiApp->signalGuiDocumentErased.connectSlot(&GuiDocumentListModel::removeGuiDocument, this);
}

QVariant GuiDocumentListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= this->rowCount())
        return {};

    const DocumentPtr& doc = m_vecGuiDocument.at(index.row())->document();
    switch (role) {
    case Qt::ToolTipRole:
        return filepathTo<QString>(filepathCanonical(doc->filePath()));
    case Qt::DisplayRole:
    case Qt::EditRole:
        return to_QString(doc->name());
    }

    return {};
}

int GuiDocumentListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return int(m_vecGuiDocument.size());
}

void GuiDocumentListModel::appendGuiDocument(const GuiDocument* guiDoc)
{
    // NOTE: don't use rowCount() as it's virtual and appendGuiDocument() is called in constructor
    //       of this class(virtual dispatch would be bypassed)
    const auto row = int(m_vecGuiDocument.size());
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

void GuiDocumentListModel::onDocumentNameChanged(const DocumentPtr& doc, const std::string& /*name*/)
{
    auto itFound = std::find_if(
        m_vecGuiDocument.cbegin(),
        m_vecGuiDocument.cend(),
        [&](const GuiDocument* guiDoc) { return guiDoc->document() == doc; }
    );
    if (itFound != m_vecGuiDocument.cend()) {
        const int row = itFound - m_vecGuiDocument.begin();
        const QModelIndex itemIndex = this->index(row);
        emit this->dataChanged(itemIndex, itemIndex, { Qt::DisplayRole, Qt::EditRole });
    }
}

} // namespace Mayo
