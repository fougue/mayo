/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_application.h"

#include "../base/application.h"
#include "../base/application_item_selection_model.h"
#include "../base/document.h"
#include "gui_document.h"

#include <unordered_set>

namespace Mayo {

GuiApplication::GuiApplication(QObject *parent)
    : QObject(parent)
{
    m_selectionModel = new ApplicationItemSelectionModel(this);

    auto app = Application::instance().get();
    QObject::connect(
                app, &Application::documentAdded,
                this, &GuiApplication::onDocumentAdded);
    QObject::connect(
                app, &Application::documentAboutToClose,
                this, &GuiApplication::onDocumentAboutToClose);

    QObject::connect(
                m_selectionModel, &ApplicationItemSelectionModel::changed,
                this, &GuiApplication::onApplicationItemSelectionChanged);
    QObject::connect(
                m_selectionModel, &ApplicationItemSelectionModel::cleared,
                this, &GuiApplication::onApplicationItemSelectionCleared);
}

GuiApplication* GuiApplication::instance()
{
    static GuiApplication app;
    return &app;
}

GuiApplication::~GuiApplication()
{
    for (GuiDocument* guiDoc : m_vecGuiDocument)
        delete guiDoc;
}

GuiDocument* GuiApplication::findGuiDocument(const DocumentPtr& doc) const
{
    for (GuiDocument* guiDoc : m_vecGuiDocument) {
        if (guiDoc->document() == doc)
            return guiDoc;
    }

    return nullptr;
}

ApplicationItemSelectionModel* GuiApplication::selectionModel() const
{
    return m_selectionModel;
}

void GuiApplication::onDocumentAdded(const DocumentPtr& doc)
{
    m_vecGuiDocument.push_back(new GuiDocument(doc));
    emit guiDocumentAdded(m_vecGuiDocument.back());
}

void GuiApplication::onDocumentAboutToClose(const DocumentPtr& doc)
{
    auto itFound = std::find_if(
                m_vecGuiDocument.begin(),
                m_vecGuiDocument.end(),
                [=](const GuiDocument* guiDoc) { return guiDoc->document() == doc; });
    if (itFound != m_vecGuiDocument.end()) {
        const GuiDocument* guiDoc = *itFound;
        delete guiDoc;
        m_vecGuiDocument.erase(itFound);
        emit guiDocumentErased(guiDoc);
    }
}

void GuiApplication::onApplicationItemSelectionCleared()
{
    for (GuiDocument* guiDoc : m_vecGuiDocument) {
        guiDoc->clearItemSelection();
        guiDoc->updateV3dViewer();
    }
}

void GuiApplication::onApplicationItemSelectionChanged(
        Span<ApplicationItem> selected, Span<ApplicationItem> deselected)
{
    std::unordered_set<GuiDocument*> setGuiDocDirty;
    auto funcToggleItemSelected = [&](const ApplicationItem& item) {
        GuiDocument* guiDoc = this->findGuiDocument(item.document());
        if (guiDoc) {
            guiDoc->toggleItemSelected(item);
            setGuiDocDirty.insert(guiDoc);
        }
    };
    for (const ApplicationItem& item : selected)
        funcToggleItemSelected(item);

    for (const ApplicationItem& item : deselected)
        funcToggleItemSelected(item);

    for (GuiDocument* guiDoc : setGuiDocDirty)
        guiDoc->updateV3dViewer();
}

} // namespace Mayo
