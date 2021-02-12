/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/application_item_selection_model.h"
#include "../base/span.h"
#include "../graphics/graphics_object_driver_table.h"
#include "../graphics/graphics_tree_node_mapping_driver_table.h"
#include "gui_document.h"

#include <QtCore/QObject>
#include <memory>

namespace Mayo {

class GuiDocument;

class GuiApplication : public QObject {
    Q_OBJECT
public:
    GuiApplication(const ApplicationPtr& app);
    ~GuiApplication();

    const ApplicationPtr& application() const { return m_app; }

    Span<GuiDocument*> guiDocuments() { return m_vecGuiDocument; }
    Span<const GuiDocument* const> guiDocuments() const { return m_vecGuiDocument; }
    GuiDocument* findGuiDocument(const DocumentPtr& doc) const;

    ApplicationItemSelectionModel* selectionModel() const;

    GraphicsObjectDriverTable* graphicsObjectDriverTable() const;
    GraphicsTreeNodeMappingDriverTable* graphicsTreeNodeMappingDriverTable() const;

signals:
    void guiDocumentAdded(GuiDocument* guiDoc);
    void guiDocumentErased(GuiDocument* guiDoc);

protected:
    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);

private:
    friend class GuiDocument;
    void connectApplicationItemSelectionChanged(bool on);
    void onApplicationItemSelectionCleared();
    void onApplicationItemSelectionChanged(
            Span<ApplicationItem> selected, Span<ApplicationItem> deselected);

    ApplicationPtr m_app;
    std::vector<GuiDocument*> m_vecGuiDocument;
    ApplicationItemSelectionModel* m_selectionModel = nullptr;
    std::unique_ptr<GraphicsObjectDriverTable> m_gfxObjectDriverTable;
    std::unique_ptr<GraphicsTreeNodeMappingDriverTable> m_gfxTreeNodeMappingDriverTable;
    QMetaObject::Connection m_connApplicationItemSelectionChanged;
};

} // namespace Mayo
