/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include "property.h"

#include <QtWidgets/QWidget>
class QTreeWidgetItem;

namespace Mayo {

class Document;
class DocumentItem;
class XdeDocumentItem;

class WidgetApplicationTree : public QWidget
{
    Q_OBJECT

public:
    WidgetApplicationTree(QWidget* widget = nullptr);
    ~WidgetApplicationTree();

    std::vector<DocumentItem*> selectedDocumentItems() const;
    std::vector<HandleProperty> propertiesOfCurrentObject() const;

signals:
    void selectionChanged();

private:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);
    void onDocumentItemAdded(DocumentItem* docItem);
    void onDocumentItemPropertyChanged(
            const DocumentItem* docItem, const Property* prop);

    void onTreeWidgetDocumentSelectionChanged();

    QTreeWidgetItem* loadDocumentItem(DocumentItem* docItem);
    void loadXdeShapeStructure(
            QTreeWidgetItem* treeDocItem, const XdeDocumentItem* xdeDocItem);

    QTreeWidgetItem* findTreeItemDocument(const Document* doc) const;
    QTreeWidgetItem* findTreeItemDocumentItem(const DocumentItem* docItem) const;

    class Ui_WidgetApplicationTree* m_ui = nullptr;
};

} // namespace Mayo
