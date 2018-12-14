/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item_selection_model.h"
#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Document;
class DocumentItem;
class GuiDocument;

class GuiApplication : public QObject {
    Q_OBJECT
public:
    static GuiApplication* instance();
    ~GuiApplication();

    std::vector<GuiDocument*> guiDocuments() const;
    GuiDocument* findGuiDocument(const Document* doc) const;

    ApplicationItemSelectionModel* selectionModel() const;

signals:
    void guiDocumentAdded(GuiDocument* guiDoc);
    void guiDocumentErased(const GuiDocument* guiDoc);

protected:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);

private:
    void onApplicationItemSelectionCleared();
    void onApplicationItemSelectionChanged(
            Span<ApplicationItem> selected, Span<ApplicationItem> deselected);

    GuiApplication(QObject* parent = nullptr);

    struct Doc_GuiDoc {
        Document* doc;
        GuiDocument* guiDoc;
    };

    std::vector<Doc_GuiDoc> m_vecDocGuiDoc;
    ApplicationItemSelectionModel* m_selectionModel = nullptr;
};

} // namespace Mayo
