/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_item_selection_model.h"
#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class GuiDocument;

class GuiApplication : public QObject {
    Q_OBJECT
public:
    static GuiApplication* instance();
    ~GuiApplication();

    Span<GuiDocument*> guiDocuments() { return m_vecGuiDocument; }
    Span<const GuiDocument* const> guiDocuments() const { return m_vecGuiDocument; }
    GuiDocument* findGuiDocument(const DocumentPtr& doc) const;

    ApplicationItemSelectionModel* selectionModel() const;

signals:
    void guiDocumentAdded(GuiDocument* guiDoc);
    void guiDocumentErased(const GuiDocument* guiDoc);

protected:
    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);

private:
    void onApplicationItemSelectionCleared();
    void onApplicationItemSelectionChanged(
            Span<ApplicationItem> selected, Span<ApplicationItem> deselected);

    GuiApplication(QObject* parent = nullptr);

    std::vector<GuiDocument*> m_vecGuiDocument;
    ApplicationItemSelectionModel* m_selectionModel = nullptr;
};

} // namespace Mayo
