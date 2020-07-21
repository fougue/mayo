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
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Mayo {

class ApplicationItem;
class V3dViewCameraAnimation;

class GuiDocument : public QObject {
    Q_OBJECT
public:
    GuiDocument(const DocumentPtr& doc);

    const DocumentPtr& document() const { return m_document; }
    const Handle_V3d_View& v3dView() const { return m_v3dView; }
    const Handle_AIS_InteractiveContext& aisInteractiveContext() const { return m_aisContext; }
    const Bnd_Box& gpxBoundingBox() const { return m_gpxBoundingBox; }

    GraphicsEntity findGraphicsEntity(TreeNodeId entityTreeNodeId) const;

    std::vector<GraphicsOwnerPtr> selectedGraphicsOwners() const;
    void toggleItemSelected(const ApplicationItem& appItem);
    void clearItemSelection();

    bool isOriginTrihedronVisible() const;
    void toggleOriginTrihedronVisibility();

    void updateV3dViewer();

    void processAction(const GraphicsOwnerPtr& graphicsOwner);

    V3dViewCameraAnimation* viewCameraAnimation() const { return m_cameraAnimation; }
    void setViewCameraOrientation(V3d_TypeOfOrientation projection);
    void runViewCameraAnimation(const std::function<void(Handle_V3d_View)>& fnViewChange);
    void stopViewCameraAnimation();

    enum class ViewTrihedronMode {
        None,
        V3dViewZBuffer,
        AisViewCube // Requires OpenCascade >= v7.4.0
    };
    ViewTrihedronMode viewTrihedronMode() const { return m_viewTrihedronMode; }
    void setViewTrihedronMode(ViewTrihedronMode mode);

    Qt::Corner viewTrihedronCorner() const { return m_viewTrihedronCorner; }
    void setViewTrihedronCorner(Qt::Corner corner);

    int aisViewCubeBoundingSize() const;

signals:
    void gpxBoundingBoxChanged(const Bnd_Box& bndBox);
    void viewTrihedronModeChanged(ViewTrihedronMode mode);
    void viewTrihedronCornerChanged(Qt::Corner corner);

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

    void v3dViewTrihedronDisplay(Qt::Corner corner);

    DocumentPtr m_document;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveContext m_aisContext;
    Handle_AIS_InteractiveObject m_aisOriginTrihedron;

    V3dViewCameraAnimation* m_cameraAnimation;
    ViewTrihedronMode m_viewTrihedronMode = ViewTrihedronMode::None;
    Qt::Corner m_viewTrihedronCorner = Qt::BottomLeftCorner;
    Handle_AIS_InteractiveObject m_aisViewCube;

    std::vector<GraphicsItem> m_vecGraphicsItem;
    Bnd_Box m_gpxBoundingBox;
};

} // namespace Mayo
