/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_list_model.h"

#include "application.h"
#include "document.h"

namespace Mayo {

DocumentListModel::DocumentListModel(Application *app)
    : QStringListModel(app)
{
    for (Document* doc : app->documents())
        this->appendDocument(doc);
    QObject::connect(
                app, &Application::documentAdded,
                this, &DocumentListModel::appendDocument);
    QObject::connect(
                app, &Application::documentErased,
                this, &DocumentListModel::removeDocument);
}

QVariant DocumentListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::ToolTipRole)
        return m_docs.at(index.row())->filePath();
    return QStringListModel::data(index, role);
}

void DocumentListModel::appendDocument(const Document *doc)
{
    const int rowId = this->rowCount();
    this->insertRow(rowId);
    const QModelIndex indexRow = this->index(rowId);
    this->setData(indexRow, doc->label());
    m_docs.emplace_back(doc);
}

void DocumentListModel::removeDocument(const Document *doc)
{
    auto itFound = std::find(m_docs.begin(), m_docs.end(), doc);
    if (itFound != m_docs.end()) {
        this->removeRow(itFound - m_docs.begin());
        m_docs.erase(itFound);
    }
}

} // namespace Mayo
