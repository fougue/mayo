/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../graphics/graphics_entity.h"
#include "../graphics/graphics_tree_node_mapping.h"

#include <QtCore/QObject>
#include <AIS_InteractiveContext.hxx>
#include <Bnd_Box.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Mayo {

class ApplicationItem;

class GuiDocument : public QObject {
    Q_OBJECT
public:
    GuiDocument(const DocumentPtr& doc);

    const DocumentPtr& document() const { return m_document; }
    const Handle_V3d_View& v3dView() const { return m_v3dView; }
    const Handle_AIS_InteractiveContext& aisInteractiveContext() const { return m_aisContext; }
    const Bnd_Box& gpxBoundingBox() const { return m_gpxBoundingBox; }

    std::vector<GraphicsOwnerPtr> selectedGraphicsOwners() const;
    void toggleItemSelected(const ApplicationItem& appItem);
    void clearItemSelection();

    bool isOriginTrihedronVisible() const;
    void toggleOriginTrihedronVisibility();

    void updateV3dViewer();

signals:
    void gpxBoundingBoxChanged(const Bnd_Box& bndBox);

private:
    void onDocumentEntityAdded(TreeNodeId entityTreeNodeId);
    void onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId);

    void mapGraphics(TreeNodeId entityTreeNodeId);

    struct GraphicsItem {
        GraphicsEntity graphicsEntity;
        TreeNodeId entityTreeNodeId;
        std::unique_ptr<GraphicsTreeNodeMapping> gpxTreeNodeMapping;
    };

    const GraphicsItem* findGraphicsItem(TreeNodeId entityTreeNodeId) const;

    DocumentPtr m_document;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveContext m_aisContext;
    Handle_AIS_InteractiveObject m_aisOriginTrihedron;
    std::vector<GraphicsItem> m_vecGraphicsItem;
    Bnd_Box m_gpxBoundingBox;
};

} // namespace Mayo
