/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/application_item_selection_model.h"
#include "../graphics/graphics_object_driver.h"
#include "gui_document.h"

#include <gsl/span>
#include <memory>

namespace Mayo {

class GuiDocument;

// Provides management of GuiDocument objects
//
// GuiApplication is connected to a "base" Application object:
//     - when a Base::Document is created, a corresponding GuiDocument object is automatically created
//     - when a Base::Document is closed, the mapped GuiDocument is automatically destroyed
//
// Typically application code should not create/destroy GuiDocument(s), this is the
// responsability of a central GuiApplication object. In some corner-case scenarios though, this
// behavior can be switched off with GuiApplication::setAutomaticDocumentMapping(false)
//
// GuiApplication acts also as a container of GraphicsObjectDriver objects.
// Those drivers are used by GuiDocument to automatically create 3D graphics representing the
// entities owned by the Base::Document
class GuiApplication {
public:
    GuiApplication(const ApplicationPtr& app);
    ~GuiApplication();

    // Not copyable
    GuiApplication(const GuiApplication&) = delete;
    GuiApplication& operator=(const GuiApplication&) = delete;

    const ApplicationPtr& application() const;

    gsl::span<GuiDocument*> guiDocuments();
    gsl::span<GuiDocument* const> guiDocuments() const;
    GuiDocument* findGuiDocument(const DocumentPtr& doc) const;

    ApplicationItemSelectionModel* selectionModel() const;

    void addGraphicsObjectDriver(GraphicsObjectDriverPtr ptr);
    void addGraphicsObjectDriver(std::unique_ptr<GraphicsObjectDriver> ptr);
    gsl::span<const GraphicsObjectDriverPtr> graphicsObjectDrivers() const;
    GraphicsObjectPtr createGraphicsObject(const TDF_Label& label) const;
    GraphicsObjectDriverPtr findCompatibleGraphicsObjectDriver(const TDF_Label& label) const;

    // Whether a GuiDocument object is automatically created once a Document is added in Application
    bool automaticDocumentMapping() const;
    void setAutomaticDocumentMapping(bool on);

    // Signals
    mutable Signal<GuiDocument*> signalGuiDocumentAdded;
    mutable Signal<GuiDocument*> signalGuiDocumentErased;
    mutable Signal<GuiDocument*, const GuiDocument::MapVisibilityByTreeNodeId&> signalGuiDocumentNodesVisibilityChanged;
    mutable Signal<GuiDocument*, const Bnd_Box&> signalGuiDocumentGraphicsBoundingBoxChanged;
    mutable Signal<GuiDocument*, GuiDocument::ViewTrihedronMode> signalGuiDocumentViewTrihedronModeChanged;
    mutable Signal<GuiDocument*, Aspect_TypeOfTriedronPosition> signalGuiDocumentViewTrihedronCornerChanged;
    mutable Signal<GuiDocument*, bool> signalGuiDocumentOriginTrihedronVisibilityToggled;

protected:
    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentClosed(const DocumentPtr& doc);

private:
    friend class GuiDocument;
    void connectApplicationItemSelectionChanged(bool on);

    struct Private;
    Private* const d = nullptr;
};

} // namespace Mayo
