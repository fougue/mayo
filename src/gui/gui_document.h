/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../base/global.h"
#include "../base/signal.h"
#include "../graphics/graphics_object_driver.h"
#include "../graphics/graphics_scene.h"
#include "../graphics/graphics_view_ptr.h"
#include "v3d_view_camera_animation.h"

#include <Aspect_TypeOfTriedronPosition.hxx>
#include <Bnd_Box.hxx>
#include <V3d_View.hxx>
#include <functional>
#include <unordered_map>
#include <vector>

namespace Mayo {

class GuiApplication;
class V3dViewCameraAnimation;

// Provides the link between Base::Document and graphical representations(called "graphics objects")
class GuiDocument {
public:
    // Applicable flags for function graphicsBoundingBox()
    enum GraphicsBoundingBoxFlag {
        AllGraphics = 0xFF,
        OnlyVisibleGraphics = 0x01,
        OnlySelectedGraphics = 0x02
    };
    using GraphicsBoundingBoxFlags = unsigned;

    // Constructor & destructor
    GuiDocument(const DocumentPtr& doc, GuiApplication* guiApp);
    ~GuiDocument();

    // GuiDocument objects are not copyable
    GuiDocument(const GuiDocument&) = delete;
    GuiDocument& operator=(const GuiDocument&) = delete;

    // Gets the base document linked to
    const DocumentPtr& document() const { return m_document; }
    Document::Identifier documentIdentifier() const;
    static Document::Identifier documentIdentifier(const GuiDocument* guiDoc);

    // Gets the owning GuiApplication object
    GuiApplication* guiApplication() const { return m_guiApp; }

    // Gets the main 3D graphics view
    const OccHandle<V3d_View>& v3dView() const { return m_v3dView; }
    GraphicsViewPtr graphicsView() { return GraphicsViewPtr{ &m_gfxScene, m_v3dView }; }

    // Gets the graphics scene, container of all document's graphics objects
    GraphicsScene* graphicsScene() { return &m_gfxScene; }

    // Returns the bounding box of all graphics objects satisfying `flags`
    Bnd_Box graphicsBoundingBox(GraphicsBoundingBoxFlags flags = AllGraphics) const;

    // Gets/sets the ratio between physical pixels and device-independent pixels for the target window.
    // This value is dependent on the screen the window is on, and may have to be updated when the
    // target window is moved.
    // Common values are 1.0(default) on normal displays and 2.0 on Apple "retina" displays
    double devicePixelRatio() const { return m_devicePixelRatio; }
    void setDevicePixelRatio(double ratio);

    // Executes callback 'fn' on all graphics objects associated to tree node 'nodeId'
    // This also includes all children(deep node traversal)
    void foreachGraphicsObject(TreeNodeId nodeId, const std::function<void(GraphicsObjectPtr)>& fn) const;

    // Finds the tree node id associated to graphics object
    TreeNodeId nodeFromGraphicsObject(const GraphicsObjectPtr& gfxObject) const;

    // Toggles selected status of a tree node(doesn't affect Application's selection model)
    void toggleNodeSelected(TreeNodeId nodeId);

    // Sets selected status of a tree node(doesn't affect Application's selection model)
    void setNodeSelected(TreeNodeId nodeId, bool on);

    // Executes action associated to a 3D sensitive item
    bool processAction(const GraphicsOwnerPtr& gfxOwner);

    // -- Display mode
    int activeDisplayMode(const GraphicsObjectDriverPtr& driver) const;
    void setActiveDisplayMode(const GraphicsObjectDriverPtr& driver, int mode);

    // -- Visible state of document's tree nodes
    CheckState nodeVisibleState(TreeNodeId nodeId) const;
    void setNodeVisible(TreeNodeId nodeId, bool on);

    // -- Exploding
    double explodingFactor() const { return m_explodingFactor; }
    void setExplodingFactor(double t); // Must be in [0,1]

    // -- Visibility of trihedron at world origin
    bool isOriginTrihedronVisible() const;
    void toggleOriginTrihedronVisibility();

    // -- Camera animation

    // Flags relevant to setViewCameraOrientation() function
    enum ViewOrientationFlag {
        // No flags
        ViewOrientationFlag_None = 0x0,
        // Do view "fit all" after camera orientation change
        ViewOrientationFlag_FitAll = 0x1,
        // Find closest UP vector when changing camera orientation
        ViewOrientationFlag_FindClosestUp = 0x2,
        // All flags ON
        ViewOrientationFlag_All = 0xFF
    };
    using ViewOrientationFlags = unsigned;

    V3dViewCameraAnimation* viewCameraAnimation() const { return m_cameraAnimation; }
    void setViewCameraOrientation(V3d_TypeOfOrientation projection, ViewOrientationFlags flags = 0);
    void runViewCameraAnimation(const V3dViewCameraAnimation::ViewFunction& fnViewChange);
    void stopViewCameraAnimation();

    // -- View trihedron
    enum class ViewTrihedronMode {
        None,
        V3dViewZBuffer,
        AisViewCube // Requires OpenCascade >= v7.4.0
    };
    ViewTrihedronMode viewTrihedronMode() const { return m_viewTrihedronMode; }
    void setViewTrihedronMode(ViewTrihedronMode mode);

    Aspect_TypeOfTriedronPosition viewTrihedronCorner() const { return m_viewTrihedronCorner; }
    void setViewTrihedronCorner(Aspect_TypeOfTriedronPosition corner);

    int aisViewCubeBoundingSize() const;
    static bool isAisViewCubeObject(const GraphicsObjectPtr& gfxObject);

    // -- Background
    struct GradientBackground {
        Quantity_Color color1;
        Quantity_Color color2;
        Aspect_GradientFillMethod fillStyle;
    };
    static const GradientBackground& defaultGradientBackground();
    static void setDefaultGradientBackground(const GradientBackground& gradientBkgnd);

    // Signals
    using MapVisibilityByTreeNodeId = std::unordered_map<TreeNodeId, CheckState>;
    mutable Signal<const MapVisibilityByTreeNodeId&> signalNodesVisibilityChanged;
    mutable Signal<const Bnd_Box&> signalGraphicsBoundingBoxChanged;
    mutable Signal<ViewTrihedronMode> signalViewTrihedronModeChanged;
    mutable Signal<Aspect_TypeOfTriedronPosition> signalViewTrihedronCornerChanged;
    mutable Signal<bool> signalOriginTrihedronVisibilityToggled;

    // -- Implementation
private:
    void onDocumentEntityAdded(TreeNodeId entityTreeNodeId);
    void onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId);
    void onGraphicsSelectionChanged();

    void mapEntity(TreeNodeId entityTreeNodeId);
    void unmapEntity(TreeNodeId entityTreeNodeId);

    struct GraphicsEntity {
        struct Object {
            Object(const GraphicsObjectPtr& p) : ptr(p) {}
            GraphicsObjectPtr ptr;
            gp_Trsf trsfOriginal;
            Bnd_Box bndBox;
        };

        TreeNodeId treeNodeId;
        std::vector<Object> vecObject;
        std::unordered_map<TreeNodeId, GraphicsObjectPtr> mapTreeNodeGfxObject;
        std::unordered_map<GraphicsObjectPtr, TreeNodeId> mapGfxObjectTreeNode;
        Bnd_Box bndBox;
    };

    const GraphicsEntity* findGraphicsEntity(TreeNodeId entityTreeNodeId) const;

    void applyExplodingFactor(const GraphicsEntity& entity, double t);

    void v3dViewTrihedronDisplay(Aspect_TypeOfTriedronPosition corner);
    void configureViewCubeSizes();

    GuiApplication* m_guiApp = nullptr;
    DocumentPtr m_document;
    GraphicsScene m_gfxScene;
    OccHandle<V3d_View> m_v3dView;
    OccHandle<AIS_InteractiveObject> m_aisOriginTrihedron;
    double m_devicePixelRatio = 1.;

    V3dViewCameraAnimation* m_cameraAnimation = nullptr;
    ViewTrihedronMode m_viewTrihedronMode = ViewTrihedronMode::None;
    Aspect_TypeOfTriedronPosition m_viewTrihedronCorner = Aspect_TOTP_LEFT_UPPER;
    OccHandle<AIS_InteractiveObject> m_aisViewCube;

    std::vector<GraphicsEntity> m_vecGraphicsEntity;
    Bnd_Box m_gfxBoundingBox;

    std::unordered_map<GraphicsObjectDriverPtr, int> m_mapGfxDriverDisplayMode;
    std::unordered_map<TreeNodeId, CheckState> m_mapTreeNodeCheckState;

    double m_explodingFactor = 0.;
};

} // namespace Mayo
