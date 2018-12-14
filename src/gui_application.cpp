/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_application.h"

#include "application.h"
#include "application_item_selection_model.h"
#include "document.h"
#include "gui_document.h"

#include <unordered_set>

namespace Mayo {

GuiApplication::GuiApplication(QObject *parent)
    : QObject(parent)
{
    m_selectionModel = new ApplicationItemSelectionModel(this);

    auto app = Application::instance();
    QObject::connect(
                app, &Application::documentAdded,
                this, &GuiApplication::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &GuiApplication::onDocumentErased);

    QObject::connect(
                m_selectionModel, &ApplicationItemSelectionModel::changed,
                this, &GuiApplication::onApplicationItemSelectionChanged);
    QObject::connect(
                m_selectionModel, &ApplicationItemSelectionModel::cleared,
                this, &GuiApplication::onApplicationItemSelectionCleared);
}

GuiApplication *GuiApplication::instance()
{
    static GuiApplication app;
    return &app;
}

GuiApplication::~GuiApplication()
{
    for (Doc_GuiDoc& pair : m_vecDocGuiDoc) {
        delete pair.guiDoc;
        pair.guiDoc = nullptr;
    }
}

GuiDocument *GuiApplication::findGuiDocument(const Document *doc) const
{
    for (const Doc_GuiDoc& pair : m_vecDocGuiDoc) {
        if (pair.doc == doc)
            return pair.guiDoc;
    }
    return nullptr;
}

std::vector<GuiDocument*> GuiApplication::guiDocuments() const
{
    std::vector<GuiDocument*> vecGuiDoc;
    vecGuiDoc.reserve(m_vecDocGuiDoc.size());
    for (const Doc_GuiDoc& pair : m_vecDocGuiDoc)
        vecGuiDoc.push_back(pair.guiDoc);
    return vecGuiDoc;
}

ApplicationItemSelectionModel* GuiApplication::selectionModel() const
{
    return m_selectionModel;
}

void GuiApplication::onDocumentAdded(Document *doc)
{
    const Doc_GuiDoc pair = { doc, new GuiDocument(doc) }; // TODO: set container widget
    m_vecDocGuiDoc.emplace_back(std::move(pair));
    emit guiDocumentAdded(pair.guiDoc);
}

void GuiApplication::onDocumentErased(const Document *doc)
{
    auto itFound = std::find_if(
                m_vecDocGuiDoc.begin(),
                m_vecDocGuiDoc.end(),
                [=](const Doc_GuiDoc& pair) { return pair.doc == doc; });
    if (itFound != m_vecDocGuiDoc.end()) {
        const GuiDocument* guiDoc = itFound->guiDoc;
        delete guiDoc;
        m_vecDocGuiDoc.erase(itFound);
        emit guiDocumentErased(guiDoc);
    }
}

void GuiApplication::onApplicationItemSelectionCleared()
{
    for (Doc_GuiDoc pair : m_vecDocGuiDoc) {
        pair.guiDoc->clearItemSelection();
        pair.guiDoc->updateV3dViewer();
    }
}

void GuiApplication::onApplicationItemSelectionChanged(
        Span<ApplicationItem> selected, Span<ApplicationItem> deselected)
{
    std::unordered_set<GuiDocument*> setGuiDocDirty;
    auto funcToggleItemSelected = [&](const ApplicationItem& item) {
        GuiDocument* guiDoc = this->findGuiDocument(item.document());
        if (guiDoc != nullptr) {
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
