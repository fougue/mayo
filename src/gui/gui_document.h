/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../base/tkernel_utils.h"
#include "../graphics/graphics_object_driver.h"
#include "../graphics/graphics_scene.h"
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
class GuiApplication;
class V3dViewCameraAnimation;

// Provides the link between Base::Document and graphical representations
class GuiDocument : public QObject {
    Q_OBJECT
public:
    GuiDocument(const DocumentPtr& doc, GuiApplication* guiApp);

    GuiApplication* guiApplication() const { return m_guiApp; }

    const DocumentPtr& document() const { return m_document; }
    const Handle_V3d_View& v3dView() const { return m_v3dView; }
    GraphicsScene* graphicsScene() { return &m_gfxScene; }
    const Bnd_Box& graphicsBoundingBox() const { return m_gfxBoundingBox; }
    void foreachGraphicsObject(TreeNodeId nodeId, const std::function<void(GraphicsObjectPtr)>& fn) const;

    TreeNodeId nodeFromGraphicsObject(const GraphicsObjectPtr& object) const;

    void toggleItemSelected(const ApplicationItem& appItem);

    int activeDisplayMode(const GraphicsObjectDriverPtr& driver) const;
    void setActiveDisplayMode(const GraphicsObjectDriverPtr& driver, int mode);

    Qt::CheckState nodeVisibleState(TreeNodeId nodeId) const;
    void setNodeVisible(TreeNodeId nodeId, bool on);

    bool isOriginTrihedronVisible() const;
    void toggleOriginTrihedronVisibility();

    bool processAction(const GraphicsOwnerPtr& graphicsOwner);

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
    void nodesVisibilityChanged(const std::unordered_map<TreeNodeId, Qt::CheckState>& mapNodeId);

    void graphicsBoundingBoxChanged(const Bnd_Box& bndBox);

    void viewTrihedronModeChanged(ViewTrihedronMode mode);
    void viewTrihedronCornerChanged(Qt::Corner corner);

private:
    void onDocumentEntityAdded(TreeNodeId entityTreeNodeId);
    void onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId);
    void onGraphicsSelectionChanged();

    void mapGraphics(TreeNodeId entityTreeNodeId);

    struct GraphicsEntity {
        TreeNodeId treeNodeId;
        std::vector<GraphicsObjectPtr> vecGfxObject;
        std::unordered_map<TreeNodeId, GraphicsObjectPtr> mapTreeNodeGfxObject;
        std::unordered_map<GraphicsObjectPtr, TreeNodeId> mapGfxObjectTreeNode;
    };

    const GraphicsEntity* findGraphicsEntity(TreeNodeId entityTreeNodeId) const;

    void v3dViewTrihedronDisplay(Qt::Corner corner);

    GuiApplication* m_guiApp = nullptr;
    DocumentPtr m_document;
    GraphicsScene m_gfxScene;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveObject m_aisOriginTrihedron;

    V3dViewCameraAnimation* m_cameraAnimation;
    ViewTrihedronMode m_viewTrihedronMode = ViewTrihedronMode::None;
    Qt::Corner m_viewTrihedronCorner = Qt::BottomLeftCorner;
    Handle_AIS_InteractiveObject m_aisViewCube;

    std::vector<GraphicsEntity> m_vecGraphicsEntity;
    Bnd_Box m_gfxBoundingBox;

    std::unordered_map<GraphicsObjectDriverPtr, int> m_mapGfxDriverDisplayMode;
    std::unordered_map<TreeNodeId, Qt::CheckState> m_mapTreeNodeCheckState;
};

} // namespace Mayo
