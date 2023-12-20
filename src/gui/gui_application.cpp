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

struct GuiApplication::Private {

    void onApplicationItemSelectionChanged(
            Span<const ApplicationItem> selected, Span<const ApplicationItem> deselected)
    {
        std::unordered_set<GuiDocument*> setGuiDocDirty;
        auto fnToggleItemSelected = [&](const ApplicationItem& item) {
            GuiDocument* guiDoc = m_backPtr->findGuiDocument(item.document());
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

    GuiApplication* m_backPtr = nullptr;
    ApplicationPtr m_app;
    std::vector<GuiDocument*> m_vecGuiDocument;
    std::vector<GraphicsObjectDriverPtr> m_vecGfxObjectDriver;
    SignalConnectionHandle m_connApplicationItemSelectionChanged;
    ApplicationItemSelectionModel m_selectionModel;
    bool m_automaticDocumentMapping = true;
};

GuiApplication::GuiApplication(const ApplicationPtr& app)
    : d(new Private)
{
    d->m_backPtr = this;
    d->m_app = app;

    app->signalDocumentAdded.connectSlot(&GuiApplication::onDocumentAdded, this);
    app->signalDocumentAboutToClose.connectSlot(&GuiApplication::onDocumentAboutToClose, this);
    this->connectApplicationItemSelectionChanged(true);
}

GuiApplication::~GuiApplication()
{
    for (GuiDocument* guiDoc : d->m_vecGuiDocument)
        delete guiDoc;

    delete d;
}

const ApplicationPtr& GuiApplication::application() const
{
    return d->m_app;
}

Span<GuiDocument*> GuiApplication::guiDocuments()
{
    return d->m_vecGuiDocument;
}

Span<GuiDocument* const> GuiApplication::guiDocuments() const
{
    return d->m_vecGuiDocument;
}

GuiDocument* GuiApplication::findGuiDocument(const DocumentPtr& doc) const
{
    for (GuiDocument* guiDoc : d->m_vecGuiDocument) {
        if (guiDoc->document() == doc)
            return guiDoc;
    }

    return nullptr;
}

ApplicationItemSelectionModel* GuiApplication::selectionModel() const
{
    return &d->m_selectionModel;
}

void GuiApplication::addGraphicsObjectDriver(GraphicsObjectDriverPtr ptr)
{
    d->m_vecGfxObjectDriver.push_back(ptr);
}

void GuiApplication::addGraphicsObjectDriver(std::unique_ptr<GraphicsObjectDriver> ptr)
{
    d->m_vecGfxObjectDriver.push_back(ptr.release()); // Will be converted to opencascade::handle<>
}

Span<const GraphicsObjectDriverPtr> GuiApplication::graphicsObjectDrivers() const
{
    return d->m_vecGfxObjectDriver;
}

GraphicsObjectPtr GuiApplication::createGraphicsObject(const TDF_Label& label) const
{
    GraphicsObjectDriver* driverPartialSupport = nullptr;
    for (const GraphicsObjectDriverPtr& driver : d->m_vecGfxObjectDriver) {
        const GraphicsObjectDriver::Support support = driver->supportStatus(label);
        if (support == GraphicsObjectDriver::Support::Complete)
            return driver->createObject(label);

        if (support == GraphicsObjectDriver::Support::Partial)
            driverPartialSupport = driver.get();
    }

    if (driverPartialSupport)
        return driverPartialSupport->createObject(label);
    else
        return {};
}

bool GuiApplication::automaticDocumentMapping() const
{
    return d->m_automaticDocumentMapping;
}

void GuiApplication::setAutomaticDocumentMapping(bool on)
{
    d->m_automaticDocumentMapping = on;
}

void GuiApplication::onDocumentAdded(const DocumentPtr& doc)
{
    if (d->m_automaticDocumentMapping) {
        d->m_vecGuiDocument.push_back(new GuiDocument(doc, this));
        this->signalGuiDocumentAdded.send(d->m_vecGuiDocument.back());
    }
}

void GuiApplication::onDocumentAboutToClose(const DocumentPtr& doc)
{
    auto itFound = std::find_if(
                d->m_vecGuiDocument.begin(),
                d->m_vecGuiDocument.end(),
                [=](const GuiDocument* guiDoc) { return guiDoc->document() == doc; }
    );
    if (itFound != d->m_vecGuiDocument.end()) {
        GuiDocument* guiDoc = *itFound;
        d->m_vecGuiDocument.erase(itFound);
        this->signalGuiDocumentErased.send(guiDoc);
        delete guiDoc;
    }
}

void GuiApplication::connectApplicationItemSelectionChanged(bool on)
{
    d->m_connApplicationItemSelectionChanged.disconnect();
    if (on) {
        auto& sigChanged = d->m_selectionModel.signalChanged;
        d->m_connApplicationItemSelectionChanged = sigChanged.connectSlot(&Private::onApplicationItemSelectionChanged, d);
    }
}

} // namespace Mayo
