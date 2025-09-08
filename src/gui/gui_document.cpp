/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document.h"

#include "../base/application.h"
#include "../base/application_item.h"
#include "../base/bnd_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/math_utils.h"
#include "../base/tkernel_utils.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_application.h"

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include <AIS_ViewCube.hxx>
#endif
#include <AIS_ConnectedInteractive.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include <cmath>

namespace Mayo {

namespace Internal {

static OccHandle<AIS_Trihedron> createOriginTrihedron()
{
    auto axis = makeOccHandle<Geom_Axis2Placement>(gp::XOY());
    auto aisTrihedron = makeOccHandle<AIS_Trihedron>(axis);
    aisTrihedron->SetDatumDisplayMode(Prs3d_DM_WireFrame);
    aisTrihedron->SetDrawArrows(false);
    aisTrihedron->Attributes()->DatumAspect()->LineAspect(Prs3d_DP_XAxis)->SetWidth(2.5);
    aisTrihedron->Attributes()->DatumAspect()->LineAspect(Prs3d_DP_YAxis)->SetWidth(2.5);
    aisTrihedron->Attributes()->DatumAspect()->LineAspect(Prs3d_DP_ZAxis)->SetWidth(2.5);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_XAxis, Quantity_NOC_RED2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_YAxis, Quantity_NOC_GREEN2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_ZAxis, Quantity_NOC_BLUE2);
    aisTrihedron->SetLabel(Prs3d_DP_XAxis, "");
    aisTrihedron->SetLabel(Prs3d_DP_YAxis, "");
    aisTrihedron->SetLabel(Prs3d_DP_ZAxis, "");
    //aisTrihedron->SetTextColor(Quantity_NOC_GRAY40);
    aisTrihedron->SetSize(60);
    aisTrihedron->SetTransformPersistence(
        new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, axis->Ax2().Location())
    );
    aisTrihedron->Attributes()->SetZLayer(Graphic3d_ZLayerId_Topmost);
    aisTrihedron->SetInfiniteState(true);
    return aisTrihedron;
}

static GuiDocument::GradientBackground& defaultGradientBackground()
{
    static GuiDocument::GradientBackground defaultGradientBackground{
        Quantity_NOC_GRAY50, Quantity_NOC_GRAY60, Aspect_GFM_VER
    };
    return defaultGradientBackground;
}

// Find the Up direction closest to current instead of `upStart`
// NOTE: excerpt from OpenCascade/src/AIS/AIS_ViewCube.cpp
static gp_Dir findClosestUpDirection(const OccHandle<Graphic3d_Camera>& camera, const gp_Dir& upStart)
{
    constexpr double pi = 3.14159265358979323846;
    const gp_Dir newDir = camera->Direction();
    const gp_Ax1 newDirAx1(gp::Origin(), newDir);
    const gp_Dir upArray[] = {
        camera->Up(),
        camera->Up().Rotated(newDirAx1, pi / 2.),
        camera->Up().Rotated(newDirAx1, pi),
        camera->Up().Rotated(newDirAx1, pi * 1.5),
    };

    double bestAngle = Precision::Infinite();
    gp_Dir upBest;
    for (const gp_Dir& up : upArray) {
        const double angle = up.Angle(upStart);
        if (bestAngle > angle) {
            bestAngle = angle;
            upBest = up;
        }
    }

    return upBest;
}

} // namespace Internal

GuiDocument::GuiDocument(const DocumentPtr& doc, GuiApplication* guiApp)
    : m_guiApp(guiApp),
      m_document(doc),
      m_v3dView(m_gfxScene.createV3dView()),
      m_aisOriginTrihedron(Internal::createOriginTrihedron()),
      m_cameraAnimation(new V3dViewCameraAnimation)
{
    Expects(!doc.IsNull());

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    this->setViewTrihedronMode(ViewTrihedronMode::AisViewCube);
    this->setViewTrihedronCorner(Aspect_TOTP_LEFT_LOWER);
#else
    this->setViewTrihedronMode(ViewTrihedronMode::V3dViewZBuffer);
    this->setViewTrihedronCorner(Aspect_TOTP_LEFT_LOWER);
#endif

    //m_v3dView->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    // 3D view - Enable anti-aliasing with MSAA
    m_v3dView->ChangeRenderingParams().IsAntialiasingEnabled = true;
    m_v3dView->ChangeRenderingParams().NbMsaaSamples = 4;
    m_v3dView->ChangeRenderingParams().CollectedStats = Graphic3d_RenderingParams::PerfCounters_Extended;
    m_v3dView->ChangeRenderingParams().StatsPosition = new Graphic3d_TransformPers(
        Graphic3d_TMF_2d, Aspect_TOTP_RIGHT_UPPER, Graphic3d_Vec2i(20, 20)
    );
    // 3D view - Set gradient background
    m_v3dView->SetBgGradientColors(
        GuiDocument::defaultGradientBackground().color1,
        GuiDocument::defaultGradientBackground().color2,
        GuiDocument::defaultGradientBackground().fillStyle
    );
    //m_v3dView->SetShadingModel(Graphic3d_TOSM_PBR);

    m_cameraAnimation->setView(m_v3dView);

    for (int i = 0; i < doc->entityCount(); ++i)
        this->mapEntity(doc->entityTreeNodeId(i));

    doc->signalEntityAdded.connectSlot(&GuiDocument::onDocumentEntityAdded, this);
    doc->signalEntityAboutToBeDestroyed.connectSlot(&GuiDocument::onDocumentEntityAboutToBeDestroyed, this);
    m_gfxScene.signalSelectionChanged.connectSlot(&GuiDocument::onGraphicsSelectionChanged, this);
}

Bnd_Box GuiDocument::graphicsBoundingBox(GraphicsBoundingBoxFlags flags) const
{
    if (flags == GraphicsBoundingBoxFlag::AllGraphics)
        return m_gfxBoundingBox;

    Bnd_Box bndBox;

    auto fnIsVisibleTreeNode = [=](TreeNodeId nodeId) {
        return (flags & OnlyVisibleGraphics) == 0 || this->nodeVisibleState(nodeId) == CheckState::On;
    };

    // Retrieve the application items being selected and visible
    auto appSelectionModel = this->guiApplication()->selectionModel();
    std::vector<ApplicationItem> appItems;
    if (flags & OnlySelectedGraphics) {
        for (const ApplicationItem& item : appSelectionModel->selectedItems()) {
            if (item.document() == this->document()
                && (!item.isDocumentTreeNode() || fnIsVisibleTreeNode(item.documentTreeNode().id())))
            {
                appItems.push_back(item);
            }
        }
    }

    // If no application items selected(and visible), then take the whole document
    if (appItems.empty())
        appItems = { ApplicationItem{this->document()} };

    // Helper function to extend main bounding box with each visible graphics object inside tree node
    auto fnAddTreeNodeBndBox = [&](TreeNodeId nodeId) {
        if (fnIsVisibleTreeNode(nodeId)) {
            this->foreachGraphicsObject(nodeId, [&](GraphicsObjectPtr gfxObject) {
                BndUtils::add(&bndBox, GraphicsUtils::AisObject_boundingBox(gfxObject));
            });
        }
    };

    // Iterate over application items to compute main bounding box
    for (const ApplicationItem& item : appItems) {
        const Tree<TDF_Label>& modelTree = item.document()->modelTree();
        if (item.isDocument())
            traverseTree(modelTree, fnAddTreeNodeBndBox);
        else
            traverseTree(item.documentTreeNode().id(), modelTree, fnAddTreeNodeBndBox);
    }

    return bndBox;
}

void GuiDocument::setDevicePixelRatio(double ratio)
{
    if (MathUtils::fuzzyEqual(m_devicePixelRatio, ratio))
        return;

    m_devicePixelRatio = ratio;
    switch (m_viewTrihedronMode) {
    case ViewTrihedronMode::None: {
        break;
    }
    case ViewTrihedronMode::V3dViewZBuffer: {
        this->v3dViewTrihedronDisplay(m_viewTrihedronCorner);
        break;
    }
    case ViewTrihedronMode::AisViewCube: {
        this->configureViewCubeSizes();
        if (m_aisViewCube)
            m_gfxScene.recomputeObjectPresentation(m_aisViewCube);
        break;
    }
    } // endswitch
}

GuiDocument::~GuiDocument()
{
    delete m_cameraAnimation;
}

Document::Identifier GuiDocument::documentIdentifier() const
{
    return m_document ? m_document->identifier() : -1;
}

Document::Identifier GuiDocument::documentIdentifier(const GuiDocument* guiDoc)
{
    return guiDoc ? guiDoc->documentIdentifier() : -1;
}

void GuiDocument::foreachGraphicsObject(
        TreeNodeId nodeId, const std::function<void (GraphicsObjectPtr)>& fn
    ) const
{
    if (!fn)
        return;

    const Tree<TDF_Label>& docModelTree = m_document->modelTree();
    const GraphicsEntity* ptrItem = this->findGraphicsEntity(docModelTree.nodeRoot(nodeId));
    if (!ptrItem)
        return;

    traverseTree(nodeId, docModelTree, [&](TreeNodeId id) {
        GraphicsObjectPtr gfxObject = CppUtils::findValue(id, ptrItem->mapTreeNodeGfxObject);
        if (gfxObject)
            fn(gfxObject);
    });
}

TreeNodeId GuiDocument::nodeFromGraphicsObject(const GraphicsObjectPtr& gfxObject) const
{
    if (!gfxObject)
        return 0;

    for (const GraphicsEntity& gfxEntity: m_vecGraphicsEntity) {
        auto it = gfxEntity.mapGfxObjectTreeNode.find(gfxObject);
        if (it != gfxEntity.mapGfxObjectTreeNode.cend())
            return it->second;
    }

    return 0;
}

void GuiDocument::toggleNodeSelected(TreeNodeId nodeId)
{
    this->foreachGraphicsObject(nodeId, [=](GraphicsObjectPtr gfxObject) {
        m_gfxScene.toggleOwnerSelected(gfxObject->GlobalSelOwner());
    });
}

void GuiDocument::setNodeSelected(TreeNodeId nodeId, bool on)
{
    this->foreachGraphicsObject(nodeId, [=](GraphicsObjectPtr gfxObject) {
        m_gfxScene.setOwnerSelected(gfxObject->GlobalSelOwner(), on);
    });
}

int GuiDocument::activeDisplayMode(const GraphicsObjectDriverPtr& driver) const
{
    if (!driver)
        return -1;

    auto itDriver = m_mapGfxDriverDisplayMode.find(driver);
    if (itDriver != m_mapGfxDriverDisplayMode.cend())
        return itDriver->second;
    else
        return driver->defaultDisplayMode();
}

void GuiDocument::setActiveDisplayMode(const GraphicsObjectDriverPtr& driver, int mode)
{
    if (!driver)
        return;

    if (this->activeDisplayMode(driver) == mode)
        return;

    m_mapGfxDriverDisplayMode.insert_or_assign(driver, mode);
    for (const TreeNodeId entityNodeId : m_document->modelTree().roots()) {
        this->foreachGraphicsObject(entityNodeId, [&](GraphicsObjectPtr object) {
            if (GraphicsObjectDriver::get(object) == driver)
                driver->applyDisplayMode(object, mode);
        });
    }
}

CheckState GuiDocument::nodeVisibleState(TreeNodeId nodeId) const
{
    auto itFound = m_mapTreeNodeCheckState.find(nodeId);
    return itFound != m_mapTreeNodeCheckState.cend() ? itFound->second : CheckState::Off;
}

void GuiDocument::setNodeVisible(TreeNodeId nodeId, bool on)
{
    auto itNode = m_mapTreeNodeCheckState.find(nodeId);
    if (itNode == m_mapTreeNodeCheckState.end())
        return; // Error: unknown tree node

    const CheckState nodeVisibleState = on ? CheckState::On : CheckState::Off;
    if (itNode->second == nodeVisibleState)
        return; // Same visible state

    // Helper data/function to keep track of all the nodes whose visibility state is altered
    std::unordered_map<TreeNodeId, CheckState> mapNodeIdVisibleState;
    auto fnSetNodeVisibleState = [&](TreeNodeId id, CheckState state) {
        auto it = m_mapTreeNodeCheckState.find(id);
        if (it == m_mapTreeNodeCheckState.cend()) {
            m_mapTreeNodeCheckState[id] = state;
            mapNodeIdVisibleState[id] = state;
        }
        else if (it->second != state) {
            it->second = state;
            mapNodeIdVisibleState[id] = state;
        }
    };

    // Recursive show/hide of the input node graphics
    const Tree<TDF_Label>& docModelTree = m_document->modelTree();
    traverseTree(nodeId, docModelTree , [=](TreeNodeId id) {
        fnSetNodeVisibleState(id, nodeVisibleState);
    });
    this->foreachGraphicsObject(nodeId, [=](GraphicsObjectPtr gfxObject) {
        GraphicsUtils::AisObject_setVisible(gfxObject, on);
    });

    // Keep selection state of the input node: in case the node graphics are "shown" back again then
    // AIS object selection status is lost
    const ApplicationItem appItem({ m_document, nodeId });
    bool isAppItemSelected = m_guiApp->selectionModel()->isSelected(appItem);
    if (!isAppItemSelected) { // Check if a parent is selected
        TreeNodeId parentId = docModelTree.nodeParent(nodeId);
        while (parentId != 0 && !isAppItemSelected) {
            const ApplicationItem parentAppItem({ m_document, parentId });
            isAppItemSelected = m_guiApp->selectionModel()->isSelected(parentAppItem);
            parentId = docModelTree.nodeParent(parentId);
        }
    }

    if (on && isAppItemSelected)
        this->setNodeSelected(nodeId, true);

    // Keep selection state of input node children
    traverseTree(nodeId, docModelTree, [=](TreeNodeId childNodeId) {
        if (childNodeId != nodeId) {
            const ApplicationItem childAppItem({ m_document, childNodeId });
            const bool isChildNodeSelected = m_guiApp->selectionModel()->isSelected(childAppItem);
            if (on && isChildNodeSelected)
                this->setNodeSelected(childNodeId, true);
        }
    });

    // Parent nodes check state
    TreeNodeId parentId = docModelTree.nodeParent(nodeId);
    while (parentId != 0) {
        int childCount = 0;
        int checkedCount = 0;
        int uncheckedCount = 0;
        visitDirectChildren(parentId, docModelTree, [&](TreeNodeId id) {
            ++childCount;
            const CheckState childState = this->nodeVisibleState(id);
            if (childState == CheckState::On)
                ++checkedCount;
            else if (childState == CheckState::Off)
                ++uncheckedCount;
        });
        CheckState parentVisibleState = CheckState::Partially;
        if (checkedCount == childCount)
            parentVisibleState = CheckState::On;
        else if (uncheckedCount == childCount)
            parentVisibleState = CheckState::Off;

        fnSetNodeVisibleState(parentId, parentVisibleState);
        parentId = docModelTree.nodeParent(parentId);
    }

    // Notify all node visibility changes
    if (!mapNodeIdVisibleState.empty())
        this->signalNodesVisibilityChanged.send(mapNodeIdVisibleState);
}

void GuiDocument::setExplodingFactor(double t)
{
    m_explodingFactor = t;
    for (const GraphicsEntity& entity : m_vecGraphicsEntity)
        applyExplodingFactor(entity, t);

    m_gfxScene.redraw();
}

bool GuiDocument::isOriginTrihedronVisible() const
{
    return m_gfxScene.isObjectVisible(m_aisOriginTrihedron);
}

void GuiDocument::toggleOriginTrihedronVisibility()
{
    const bool visible = !this->isOriginTrihedronVisible();
    m_gfxScene.setObjectVisible(m_aisOriginTrihedron, visible);
    this->signalOriginTrihedronVisibilityToggled.send(visible);
}

bool GuiDocument::processAction(const GraphicsOwnerPtr& gfxOwner)
{
    if (!gfxOwner)
        return false;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    auto viewCubeOwner = OccHandle<AIS_ViewCubeOwner>::DownCast(gfxOwner);
    if (viewCubeOwner) {
        this->setViewCameraOrientation(viewCubeOwner->MainOrientation(), ViewOrientationFlag_All);
        return true;
    }
#endif

    return false;
}

void GuiDocument::setViewCameraOrientation(V3d_TypeOfOrientation projection, ViewOrientationFlags flags)
{
    this->runViewCameraAnimation([=](OccHandle<V3d_View> view) {
        view->SetProj(projection);
        if (flags & ViewOrientationFlag_FindClosestUp) {
            const gp_Dir& upStart = m_cameraAnimation->cameraStart()->Up();
            view->Camera()->SetUp(Internal::findClosestUpDirection(view->Camera(), upStart));
        }

        if (flags & ViewOrientationFlag_FitAll) {
            GraphicsUtils::V3dView_fitAll(
                view, this->graphicsBoundingBox(OnlySelectedGraphics | OnlyVisibleGraphics)
            );
        }
    });
}

void GuiDocument::runViewCameraAnimation(const V3dViewCameraAnimation::ViewFunction& fnViewChange)
{
    m_cameraAnimation->configureCameraChange(fnViewChange);
    m_cameraAnimation->start();
}

void GuiDocument::stopViewCameraAnimation()
{
    m_cameraAnimation->stop();
}

void GuiDocument::setViewTrihedronMode(ViewTrihedronMode mode)
{
    if (mode == m_viewTrihedronMode)
        return;

    auto fnViewCubeSetVisible = [&](bool on) {
        m_gfxScene.setObjectVisible(m_aisViewCube, on);
    };

    switch (mode) {
    case ViewTrihedronMode::None: {
        m_v3dView->TriedronErase();
        fnViewCubeSetVisible(false);
        break;
    }
    case ViewTrihedronMode::V3dViewZBuffer: {
        this->v3dViewTrihedronDisplay(m_viewTrihedronCorner);
        fnViewCubeSetVisible(false);
        break;
    }
    case ViewTrihedronMode::AisViewCube: {
        if (m_aisViewCube.IsNull()) {
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
            auto aisViewCube = new AIS_ViewCube;
            m_aisViewCube = aisViewCube;
            aisViewCube->SetBoxColor(Quantity_NOC_GRAY75);
            //aisViewCube->SetFixedAnimationLoop(false);
            aisViewCube->SetAxesLabels("", "", "");
            this->configureViewCubeSizes();
            m_gfxScene.addObject(aisViewCube);
            //aisViewCube->Attributes()->DatumAspect()->LineAspect(Prs3d_DP_XAxis)->SetColor(Quantity_NOC_RED2);
            const OccHandle<Prs3d_DatumAspect>& datumAspect = aisViewCube->Attributes()->DatumAspect();
            datumAspect->ShadingAspect(Prs3d_DP_XAxis)->SetColor(Quantity_NOC_RED2);
            datumAspect->ShadingAspect(Prs3d_DP_YAxis)->SetColor(Quantity_NOC_GREEN2);
            datumAspect->ShadingAspect(Prs3d_DP_ZAxis)->SetColor(Quantity_NOC_BLUE2);
#endif
        }

        m_v3dView->TriedronErase();
        fnViewCubeSetVisible(true);
        break;
    }
    } // endswitch

    m_viewTrihedronMode = mode;
    this->signalViewTrihedronModeChanged.send(mode);
}

void GuiDocument::setViewTrihedronCorner(Aspect_TypeOfTriedronPosition corner)
{
    if (corner == m_viewTrihedronCorner)
        return;

    switch (m_viewTrihedronMode) {
    case ViewTrihedronMode::None: {
        break; // Nothing to do
    }
    case ViewTrihedronMode::V3dViewZBuffer: {
        this->v3dViewTrihedronDisplay(corner);
        break;
    }
    case ViewTrihedronMode::AisViewCube: {
        if (m_aisViewCube)
            m_aisViewCube->TransformPersistence()->SetCorner2d(corner);

        break;
    }
    } // endswitch

    m_viewTrihedronCorner = corner;
    this->signalViewTrihedronCornerChanged.send(corner);
}

int GuiDocument::aisViewCubeBoundingSize() const
{
    if (!m_aisViewCube)
        return 0;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    auto hnd = OccHandle<AIS_ViewCube>::DownCast(m_aisViewCube);
    auto size =
        2 * (hnd->Size()
             + hnd->BoxFacetExtension()
             + hnd->BoxEdgeGap()
             + hnd->BoxEdgeMinSize()
             + hnd->BoxCornerMinSize()
             + hnd->RoundRadius()
            )
        + hnd->AxesPadding()
        + hnd->FontHeight()
    ;
    return std::lround(size);
#else
    return 0;
#endif
}

bool GuiDocument::isAisViewCubeObject([[maybe_unused]] const GraphicsObjectPtr& gfxObject)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    return !OccHandle<AIS_ViewCube>::DownCast(gfxObject).IsNull();
#else
    return false;
#endif
}

const GuiDocument::GradientBackground& GuiDocument::defaultGradientBackground()
{
    return Internal::defaultGradientBackground();
}

void GuiDocument::setDefaultGradientBackground(const GradientBackground& gradientBkgnd)
{
    Internal::defaultGradientBackground() = gradientBkgnd;
}

void GuiDocument::onDocumentEntityAdded(TreeNodeId entityTreeNodeId)
{
    this->mapEntity(entityTreeNodeId);
    BndUtils::add(&m_gfxBoundingBox, m_vecGraphicsEntity.back().bndBox);
    GraphicsUtils::V3dView_fitAll(m_v3dView, this->graphicsBoundingBox(OnlySelectedGraphics | OnlyVisibleGraphics));
    this->signalGraphicsBoundingBoxChanged.send(m_gfxBoundingBox);
}

void GuiDocument::onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId)
{
    this->unmapEntity(entityTreeNodeId);
    // Recompute bounding box
    m_gfxBoundingBox.SetVoid();
    for (const GraphicsEntity& gfxEntity : m_vecGraphicsEntity)
        BndUtils::add(&m_gfxBoundingBox, gfxEntity.bndBox);

    this->signalGraphicsBoundingBoxChanged.send(m_gfxBoundingBox);
}

void GuiDocument::onGraphicsSelectionChanged()
{
    m_guiApp->connectApplicationItemSelectionChanged(false);
    auto _ = gsl::finally([=]{ m_guiApp->connectApplicationItemSelectionChanged(true); });

    ApplicationItemSelectionModel* appSelectionModel = m_guiApp->selectionModel();
    if (m_gfxScene.selectedCount() == 0) {
        appSelectionModel->clear();
        return;
    }

    std::vector<ApplicationItem> vecSelected;
    m_gfxScene.foreachSelectedOwner([&](const GraphicsOwnerPtr& gfxOwner) {
        auto gfxObject = GraphicsObjectPtr::DownCast(
            gfxOwner ? gfxOwner->Selectable() : OccHandle<SelectMgr_SelectableObject>()
        );
        const TreeNodeId nodeId = this->nodeFromGraphicsObject(gfxObject);
        if (nodeId != 0) {
            const ApplicationItem appItem({ m_document, nodeId });
            vecSelected.push_back(std::move(appItem));
        }
    });

    std::vector<ApplicationItem> vecRemoved;
    for (const ApplicationItem& appItem : appSelectionModel->selectedItems()) {
        if (appItem.document() != m_document)
            continue;

        auto it = std::find(vecSelected.cbegin(), vecSelected.cend(), appItem);
        if (it == vecSelected.cend())
            vecRemoved.push_back(appItem);
    }

    if (!vecSelected.empty())
        appSelectionModel->add(vecSelected);

    if (!vecRemoved.empty())
        appSelectionModel->remove(vecRemoved);
}

void GuiDocument::mapEntity(TreeNodeId entityTreeNodeId)
{
    const Tree<TDF_Label>& docModelTree = m_document->modelTree();
    GraphicsEntity gfxEntity;
    gfxEntity.treeNodeId = entityTreeNodeId;
    std::unordered_map<TDF_Label, GraphicsObjectPtr> mapLabelGfxProduct;

    traverseTree(entityTreeNodeId, docModelTree, [&](TreeNodeId id) {
        const TDF_Label nodeLabel = docModelTree.nodeData(id);
        if (docModelTree.nodeIsLeaf(id)) {
            GraphicsObjectPtr gfxProduct = CppUtils::findValue(nodeLabel, mapLabelGfxProduct);
            if (!gfxProduct) {
                gfxProduct = m_guiApp->createGraphicsObject(nodeLabel);
                if (!gfxProduct)
                    return;

                mapLabelGfxProduct.insert({ nodeLabel, gfxProduct });
            }

            if (!docModelTree.nodeIsRoot(id)) {
                const TreeNodeId parentNodeId = docModelTree.nodeParent(id);
                const TDF_Label parentNodeLabel = docModelTree.nodeData(parentNodeId);
                if (XCaf::isShapeReference(parentNodeLabel) && m_document->xcaf().hasShapeColor(parentNodeLabel)) {
                    // Parent node is a reference and it redefines color attribute, so the graphics
                    // can't be shared with the product
                    auto gfxObject = m_guiApp->createGraphicsObject(parentNodeLabel);
                    const TreeNodeId grandParentNodeId = docModelTree.nodeParent(parentNodeId);
                    const TopLoc_Location locGrandParentShape = XCaf::shapeAbsoluteLocation(docModelTree, grandParentNodeId);
                    gfxObject->SetLocalTransformation(locGrandParentShape);
                    gfxEntity.vecObject.push_back(gfxObject);
                }
                else {
                    auto gfxInstance = new AIS_ConnectedInteractive;
                    gfxInstance->Connect(gfxProduct, XCaf::shapeAbsoluteLocation(docModelTree, id));
                    gfxInstance->SetDisplayMode(gfxProduct->DisplayMode());
                    gfxInstance->Attributes()->SetFaceBoundaryDraw(gfxProduct->Attributes()->FaceBoundaryDraw());
                    gfxInstance->SetOwner(gfxProduct->GetOwner());
                    gfxEntity.vecObject.push_back(GraphicsObjectPtr(gfxInstance));
                }

                if (XCaf::isShapeReference(parentNodeLabel))
                    id = docModelTree.nodeParent(id);
            }
            else {
                gfxEntity.vecObject.push_back(gfxProduct);
            }

            const GraphicsEntity::Object& lastGfxObject = gfxEntity.vecObject.back();
            gfxEntity.mapTreeNodeGfxObject.insert({ id, lastGfxObject.ptr });
            gfxEntity.mapGfxObjectTreeNode.insert({ lastGfxObject.ptr, id });
        }
    });

    for (const GraphicsEntity::Object& object : gfxEntity.vecObject) {
        m_gfxScene.addObject(object.ptr);
        auto driver = GraphicsObjectDriver::get(object.ptr);
        if (driver)
            driver->applyDisplayMode(object.ptr, this->activeDisplayMode(driver));
    }

    for (GraphicsEntity::Object& object : gfxEntity.vecObject) {
        object.bndBox = GraphicsUtils::AisObject_boundingBox(object.ptr);
        object.trsfOriginal = m_gfxScene.objectTransformation(object.ptr);
        BndUtils::add(&gfxEntity.bndBox, object.bndBox);
    }

    if (!MathUtils::fuzzyIsNull(m_explodingFactor))
        this->applyExplodingFactor(gfxEntity, m_explodingFactor);

    m_gfxScene.redraw();

    traverseTree(entityTreeNodeId, docModelTree, [=](TreeNodeId id) {
        m_mapTreeNodeCheckState.insert({ id, CheckState::On });
    });

    m_vecGraphicsEntity.push_back(std::move(gfxEntity));
}

void GuiDocument::unmapEntity(TreeNodeId entityTreeNodeId)
{
    {   // Delete entity graphics
        const GraphicsEntity* ptrItem = this->findGraphicsEntity(entityTreeNodeId);
        if (!ptrItem)
            return;

        for (const GraphicsEntity::Object& object : ptrItem->vecObject)
            m_gfxScene.eraseObject(object.ptr);

        const auto indexItem = ptrItem - &m_vecGraphicsEntity.front();
        m_vecGraphicsEntity.erase(m_vecGraphicsEntity.begin() + indexItem);
        m_gfxScene.redraw();
    }

    traverseTree(entityTreeNodeId, m_document->modelTree(), [=](TreeNodeId id) {
        m_mapTreeNodeCheckState.erase(id);
    });
}

const GuiDocument::GraphicsEntity* GuiDocument::findGraphicsEntity(TreeNodeId entityTreeNodeId) const
{
    auto itFound = std::find_if(
        m_vecGraphicsEntity.cbegin(),
        m_vecGraphicsEntity.cend(),
        [=](const GraphicsEntity& item) { return item.treeNodeId == entityTreeNodeId; }
    );
    return itFound != m_vecGraphicsEntity.cend() ? &(*itFound) : nullptr;
}

void GuiDocument::applyExplodingFactor(const GraphicsEntity& entity, double t)
{
    const gp_Pnt entityCenter = BndBoxCoords::get(entity.bndBox).center();
    for (const GraphicsEntity::Object& object : entity.vecObject) {
        const gp_Vec vecDirection(entityCenter, BndBoxCoords::get(object.bndBox).center());
        gp_Trsf trsfMove;
        trsfMove.SetTranslation(2 * t * vecDirection);
        m_gfxScene.setObjectTransformation(object.ptr, trsfMove * object.trsfOriginal);
    }
}

void GuiDocument::v3dViewTrihedronDisplay(Aspect_TypeOfTriedronPosition corner)
{
    const double scale = 0.075 * m_devicePixelRatio;
    m_v3dView->TriedronDisplay(corner, Quantity_NOC_GRAY50, scale, V3d_ZBUFFER);
}

void GuiDocument::configureViewCubeSizes()
{
    const int viewCubePxSize = 64;
    const int viewCubePxFontHeight = 12;
    const int viewCubePxOffsetXY = 90;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    auto viewCube = OccHandle<AIS_ViewCube>::DownCast(m_aisViewCube);
    if (viewCube) {
        viewCube->SetSize(viewCubePxSize * m_devicePixelRatio, true/*adaptOtherParams*/);
        viewCube->SetFontHeight(viewCubePxFontHeight * m_devicePixelRatio);
        const int offsetXY = std::lround(viewCubePxOffsetXY * m_devicePixelRatio);
        viewCube->SetTransformPersistence(
            new Graphic3d_TransformPers(
                Graphic3d_TMF_TriedronPers,
                m_viewTrihedronCorner,
                Graphic3d_Vec2i{offsetXY, offsetXY}
            )
        );
    }
#endif
}

} // namespace Mayo
