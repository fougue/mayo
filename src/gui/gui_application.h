/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/application_item_selection_model.h"
#include "../graphics/graphics_tree_node_mapping.h"
#include "gui_document.h"
#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Application;
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

    using GraphicsTreeNodeMappingDriverPtr = std::unique_ptr<GraphicsTreeNodeMappingDriver>;
    void addGraphicsTreeNodeMappingDriver(GraphicsTreeNodeMappingDriverPtr driver);
    Span<const GraphicsTreeNodeMappingDriverPtr> graphicsTreeNodeMappingDrivers() const;

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

    ApplicationPtr m_app;
    std::vector<GuiDocument*> m_vecGuiDocument;
    std::vector<GraphicsTreeNodeMappingDriverPtr> m_vecGraphicsTreeNodeMappingDriver;
    ApplicationItemSelectionModel* m_selectionModel = nullptr;
};

} // namespace Mayo
