/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
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

GuiApplication::GuiApplication(const ApplicationPtr& app)
    : QObject(app.get()),
      m_app(app),
      m_selectionModel(new ApplicationItemSelectionModel(this)),
      m_gfxObjectDriverTable(new GraphicsObjectDriverTable),
      m_gfxTreeNodeMappingDriverTable(new GraphicsTreeNodeMappingDriverTable)
{
    QObject::connect(
                app.get(), &Application::documentAdded,
                this, &GuiApplication::onDocumentAdded);
    QObject::connect(
                app.get(), &Application::documentAboutToClose,
                this, &GuiApplication::onDocumentAboutToClose);
    this->connectApplicationItemSelectionChanged(true);
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

GraphicsObjectDriverTable* GuiApplication::graphicsObjectDriverTable() const
{
    return m_gfxObjectDriverTable.get();
}

GraphicsTreeNodeMappingDriverTable* GuiApplication::graphicsTreeNodeMappingDriverTable() const
{
    return m_gfxTreeNodeMappingDriverTable.get();
}

void GuiApplication::onDocumentAdded(const DocumentPtr& doc)
{
    m_vecGuiDocument.push_back(new GuiDocument(doc, this));
    emit guiDocumentAdded(m_vecGuiDocument.back());
}

void GuiApplication::onDocumentAboutToClose(const DocumentPtr& doc)
{
    auto itFound = std::find_if(
                m_vecGuiDocument.begin(),
                m_vecGuiDocument.end(),
                [=](const GuiDocument* guiDoc) { return guiDoc->document() == doc; });
    if (itFound != m_vecGuiDocument.end()) {
        GuiDocument* guiDoc = *itFound;
        m_vecGuiDocument.erase(itFound);
        emit guiDocumentErased(guiDoc);
        delete guiDoc;
    }
}

void GuiApplication::connectApplicationItemSelectionChanged(bool on)
{
    if (on) {
        m_connApplicationItemSelectionChanged = QObject::connect(
                    m_selectionModel, &ApplicationItemSelectionModel::changed,
                    this, &GuiApplication::onApplicationItemSelectionChanged,
                    Qt::UniqueConnection);
    }
    else {
        QObject::disconnect(m_connApplicationItemSelectionChanged);
    }
}

void GuiApplication::onApplicationItemSelectionChanged(
        Span<ApplicationItem> selected, Span<ApplicationItem> deselected)
{
    std::unordered_set<GuiDocument*> setGuiDocDirty;
    auto fnToggleItemSelected = [&](const ApplicationItem& item) {
        GuiDocument* guiDoc = this->findGuiDocument(item.document());
        if (guiDoc) {
            guiDoc->toggleItemSelected(item);
            setGuiDocDirty.insert(guiDoc);
        }
    };
    for (const ApplicationItem& item : selected)
        fnToggleItemSelected(item);

    for (const ApplicationItem& item : deselected)
        fnToggleItemSelected(item);

    for (GuiDocument* guiDoc : setGuiDocDirty)
        guiDoc->graphicsScene()->redraw();
}

} // namespace Mayo
