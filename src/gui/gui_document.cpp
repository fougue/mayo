/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document.h"

#include "../app/theme.h" // TODO Remove this dependency
#include "../base/application_item.h"
#include "../base/bnd_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/tkernel_utils.h"
#include "../gui/gui_application.h"
#include "../gui/qtgui_utils.h"
#include "../graphics/graphics_object_driver_table.h"
#include "../graphics/graphics_utils.h"
#include "../graphics/v3d_view_camera_animation.h"

#include <QtCore/QtDebug>
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include <AIS_ViewCube.hxx>
#endif
#include <AIS_ConnectedInteractive.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <V3d_TypeOfOrientation.hxx>

namespace Mayo {

namespace Internal {

// Defined in gui_create_gfx_driver.cpp
Handle_Graphic3d_GraphicDriver createGfxDriver();

static Handle_AIS_Trihedron createOriginTrihedron()
{
    Handle_Geom_Axis2Placement axis = new Geom_Axis2Placement(gp::XOY());
    Handle_AIS_Trihedron aisTrihedron = new AIS_Trihedron(axis);
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
                new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, axis->Ax2().Location()));
    aisTrihedron->Attributes()->SetZLayer(Graphic3d_ZLayerId_Topmost);
    aisTrihedron->SetInfiniteState(true);
    return aisTrihedron;
}

} // namespace Internal

GuiDocument::GuiDocument(const DocumentPtr& doc, GuiApplication* guiApp)
    : QObject(guiApp),
      m_guiApp(guiApp),
      m_document(doc),
      m_gfxScene(this),
      m_v3dView(m_gfxScene.createV3dView()),
      m_aisOriginTrihedron(Internal::createOriginTrihedron()),
      m_cameraAnimation(new V3dViewCameraAnimation(m_v3dView, this))
{
    Expects(!doc.IsNull());

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    this->setViewTrihedronMode(ViewTrihedronMode::AisViewCube);
    this->setViewTrihedronCorner(Qt::TopLeftCorner);
#else
    this->setViewTrihedronMode(ViewTrihedronMode::V3dViewZBuffer);
    this->setViewTrihedronCorner(Qt::BottomLeftCorner);
#endif

    //m_v3dView->SetShadingModel(V3d_PHONG);
    // 3D view - Enable anti-aliasing with MSAA
    m_v3dView->ChangeRenderingParams().IsAntialiasingEnabled = true;
    m_v3dView->ChangeRenderingParams().NbMsaaSamples = 4;
    // 3D view - Set gradient background
    m_v3dView->SetBgGradientColors(
                QtGuiUtils::toColor<Quantity_Color>(
                    mayoTheme()->color(Theme::Color::View3d_BackgroundGradientStart)),
                QtGuiUtils::toColor<Quantity_Color>(
                    mayoTheme()->color(Theme::Color::View3d_BackgroundGradientEnd)),
                Aspect_GFM_VER);

    m_cameraAnimation->setEasingCurve(QEasingCurve::OutExpo);

    for (int i = 0; i < doc->entityCount(); ++i)
        this->mapGraphics(doc->entityTreeNodeId(i));

    QObject::connect(doc.get(), &Document::entityAdded, this, &GuiDocument::onDocumentEntityAdded);
    QObject::connect(
                doc.get(), &Document::entityAboutToBeDestroyed,
                this, &GuiDocument::onDocumentEntityAboutToBeDestroyed);
    QObject::connect(
                &m_gfxScene, &GraphicsScene::selectionChanged,
                this, &GuiDocument::onGraphicsSelectionChanged);
}

void GuiDocument::foreachGraphicsObject(
        TreeNodeId nodeId, const std::function<void (GraphicsObjectPtr)>& fn) const
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

TreeNodeId GuiDocument::nodeFromGraphicsObject(const GraphicsObjectPtr& object) const
{
    if (!object)
        return 0;

    for (const GraphicsEntity& gfxEntity: m_vecGraphicsEntity) {
        auto it = gfxEntity.mapGfxObjectTreeNode.find(object);
        if (it != gfxEntity.mapGfxObjectTreeNode.cend())
            return it->second;
    }

    return 0;
}

void GuiDocument::toggleItemSelected(const ApplicationItem& appItem)
{
    const DocumentPtr doc = appItem.document();
    if (doc != this->document())
        return;

    if (appItem.isDocumentTreeNode()) {
        const DocumentTreeNode& docTreeNode = appItem.documentTreeNode();
        const TreeNodeId entityNodeId = doc->modelTree().nodeRoot(docTreeNode.id());
        const GraphicsEntity* gfxEntity = this->findGraphicsEntity(entityNodeId);
        if (!gfxEntity)
            return;

        traverseTree(docTreeNode.id(), doc->modelTree(), [=](TreeNodeId id) {
            GraphicsObjectPtr gfxObject = CppUtils::findValue(id, gfxEntity->mapTreeNodeGfxObject);
            if (gfxObject)
                m_gfxScene.toggleOwnerSelection(gfxObject->GlobalSelOwner());
        });
    }
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

Qt::CheckState GuiDocument::nodeVisibleState(TreeNodeId nodeId) const
{
    auto itFound = m_mapTreeNodeCheckState.find(nodeId);
    return itFound != m_mapTreeNodeCheckState.cend() ? itFound->second : Qt::Unchecked;
}

void GuiDocument::setNodeVisible(TreeNodeId nodeId, bool on)
{
    auto itNode = m_mapTreeNodeCheckState.find(nodeId);
    if (itNode == m_mapTreeNodeCheckState.end())
        return; // Error: unknown tree node

    const Qt::CheckState nodeVisibleState = on ? Qt::Checked : Qt::Unchecked;
    if (itNode->second == nodeVisibleState)
        return; // Same visible state

    // Helper data/function to keep track of all the nodes whose visibility state are altered
    std::unordered_map<TreeNodeId, Qt::CheckState> mapNodeIdVisibleState;
    auto fnSetNodeVisibleState = [&](TreeNodeId id, Qt::CheckState state) {
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
    this->foreachGraphicsObject(nodeId, [=](GraphicsObjectPtr gfxObject){
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
        this->toggleItemSelected(appItem);

    // Keep selection state of input node children
    traverseTree(nodeId, docModelTree, [=](TreeNodeId id) {
        if (id != nodeId) {
            const ApplicationItem childAppItem({ m_document, id });
            if (on && m_guiApp->selectionModel()->isSelected(childAppItem))
                this->toggleItemSelected(childAppItem);
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
            const Qt::CheckState childState = this->nodeVisibleState(id);
            if (childState == Qt::Checked)
                ++checkedCount;
            else if (childState == Qt::Unchecked)
                ++uncheckedCount;
        });
        Qt::CheckState parentVisibleState = Qt::PartiallyChecked;
        if (checkedCount == childCount)
            parentVisibleState = Qt::Checked;
        else if (uncheckedCount == childCount)
            parentVisibleState = Qt::Unchecked;

        fnSetNodeVisibleState(parentId, parentVisibleState);
        parentId = docModelTree.nodeParent(parentId);
    }

    // Notify all node visibility changes
    if (!mapNodeIdVisibleState.empty())
        emit nodesVisibilityChanged(mapNodeIdVisibleState);
}

bool GuiDocument::isOriginTrihedronVisible() const
{
    return m_gfxScene.isObjectVisible(m_aisOriginTrihedron);
}

void GuiDocument::toggleOriginTrihedronVisibility()
{
    const bool visible = !this->isOriginTrihedronVisible();
    m_gfxScene.setObjectVisible(m_aisOriginTrihedron, visible);
}

bool GuiDocument::processAction(const GraphicsOwnerPtr& graphicsOwner)
{
    if (graphicsOwner.IsNull())
        return false;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    auto viewCubeOwner = opencascade::handle<AIS_ViewCubeOwner>::DownCast(graphicsOwner);
    if (viewCubeOwner) {
        this->setViewCameraOrientation(viewCubeOwner->MainOrientation());
        return true;
    }
#endif

    return false;
}

void GuiDocument::setViewCameraOrientation(V3d_TypeOfOrientation projection)
{
    this->runViewCameraAnimation([=](Handle_V3d_View view) {
        view->SetProj(projection);
        GraphicsUtils::V3dView_fitAll(view);
    });
}

void GuiDocument::runViewCameraAnimation(const std::function<void (Handle_V3d_View)>& fnViewChange)
{
    m_cameraAnimation->configure(fnViewChange);
    m_cameraAnimation->start(QAbstractAnimation::KeepWhenStopped);
}

void GuiDocument::stopViewCameraAnimation()
{
    m_cameraAnimation->stop();
}

static Aspect_TypeOfTriedronPosition toOccCorner(Qt::Corner corner)
{
    switch (corner) {
    case Qt::TopLeftCorner: return Aspect_TOTP_LEFT_UPPER;
    case Qt::TopRightCorner: return Aspect_TOTP_RIGHT_UPPER;
    case Qt::BottomLeftCorner: return Aspect_TOTP_LEFT_LOWER;
    case Qt::BottomRightCorner: return Aspect_TOTP_RIGHT_LOWER;
    }

    return Aspect_TOTP_LEFT_UPPER; // Fallback
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
            opencascade::handle<AIS_ViewCube> aisViewCube = new AIS_ViewCube;
            aisViewCube->SetBoxColor(Quantity_NOC_GRAY75);
            //aisViewCube->SetFixedAnimationLoop(false);
            aisViewCube->SetSize(55);
            aisViewCube->SetFontHeight(12);
            aisViewCube->SetAxesLabels("", "", "");
            aisViewCube->SetTransformPersistence(
                        new Graphic3d_TransformPers(
                            Graphic3d_TMF_TriedronPers,
                            toOccCorner(m_viewTrihedronCorner),
                            Graphic3d_Vec2i(85, 85)));
            m_gfxScene.addObject(aisViewCube);
            //aisViewCube->Attributes()->DatumAspect()->LineAspect(Prs3d_DP_XAxis)->SetColor(Quantity_NOC_RED2);
            const Handle_Prs3d_DatumAspect& datumAspect = aisViewCube->Attributes()->DatumAspect();
            datumAspect->ShadingAspect(Prs3d_DP_XAxis)->SetColor(Quantity_NOC_RED2);
            datumAspect->ShadingAspect(Prs3d_DP_YAxis)->SetColor(Quantity_NOC_GREEN2);
            datumAspect->ShadingAspect(Prs3d_DP_ZAxis)->SetColor(Quantity_NOC_BLUE2);
            m_aisViewCube = aisViewCube;
#endif
        }

        m_v3dView->TriedronErase();
        fnViewCubeSetVisible(true);
        break;
    }
    } // endswitch

    m_viewTrihedronMode = mode;
    emit this->viewTrihedronModeChanged(mode);
}

void GuiDocument::setViewTrihedronCorner(Qt::Corner corner)
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
            m_aisViewCube->TransformPersistence()->SetCorner2d(toOccCorner(corner));

        break;
    }
    } // endswitch

    m_viewTrihedronCorner = corner;
    emit this->viewTrihedronCornerChanged(corner);
}

int GuiDocument::aisViewCubeBoundingSize() const
{
    if (!m_aisViewCube)
        return 0;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    auto hnd = opencascade::handle<AIS_ViewCube>::DownCast(m_aisViewCube);
    return 2 * (hnd->Size()
                + hnd->BoxFacetExtension()
                + hnd->BoxEdgeGap()
                + hnd->BoxEdgeMinSize()
                + hnd->BoxCornerMinSize()
                + hnd->RoundRadius())
            + hnd->AxesPadding()
            + hnd->FontHeight();
#else
    return 0;
#endif
}

void GuiDocument::onDocumentEntityAdded(TreeNodeId entityTreeNodeId)
{
    this->mapGraphics(entityTreeNodeId);
    emit graphicsBoundingBoxChanged(m_gfxBoundingBox);
}

void GuiDocument::onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId)
{
    {   // Delete entity graphics
        const GraphicsEntity* ptrItem = this->findGraphicsEntity(entityTreeNodeId);
        if (!ptrItem)
            return;

        for (const GraphicsObjectPtr& gfxObject : ptrItem->vecGfxObject)
            m_gfxScene.eraseObject(gfxObject);

        const int indexItem = ptrItem - &m_vecGraphicsEntity.front();
        m_vecGraphicsEntity.erase(m_vecGraphicsEntity.begin() + indexItem);
        m_gfxScene.redraw();
    }

    traverseTree(entityTreeNodeId, m_document->modelTree(), [=](TreeNodeId id) {
        m_mapTreeNodeCheckState.erase(id);
    });

    // Recompute bounding box
    m_gfxBoundingBox.SetVoid();
    for (const GraphicsEntity& item : m_vecGraphicsEntity) {
        for (const GraphicsObjectPtr& gfxObject : item.vecGfxObject) {
            const Bnd_Box bndBox = GraphicsUtils::AisObject_boundingBox(gfxObject);
            BndUtils::add(&m_gfxBoundingBox, bndBox);
        }
    }

    emit graphicsBoundingBoxChanged(m_gfxBoundingBox);
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
                    gfxOwner ? gfxOwner->Selectable() : Handle_SelectMgr_SelectableObject());
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

void GuiDocument::mapGraphics(TreeNodeId entityTreeNodeId)
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
                gfxProduct = m_guiApp->graphicsObjectDriverTable()->createObject(nodeLabel);
                if (!gfxProduct)
                    return;

                mapLabelGfxProduct.insert({ nodeLabel, gfxProduct });
            }

            if (!docModelTree.nodeIsRoot(id)) {
                Handle_AIS_ConnectedInteractive gfxInstance = new AIS_ConnectedInteractive;
                gfxInstance->Connect(gfxProduct, XCaf::shapeAbsoluteLocation(docModelTree, id));
                gfxInstance->SetDisplayMode(gfxProduct->DisplayMode());
                gfxInstance->Attributes()->SetFaceBoundaryDraw(gfxProduct->Attributes()->FaceBoundaryDraw());
                gfxInstance->SetOwner(gfxProduct->GetOwner());
                gfxEntity.vecGfxObject.push_back(gfxInstance);
                if (XCaf::isShapeReference(docModelTree.nodeData(docModelTree.nodeParent(id))))
                    id = docModelTree.nodeParent(id);
            }
            else {
                gfxEntity.vecGfxObject.push_back(gfxProduct);
            }

            gfxEntity.mapTreeNodeGfxObject.insert({ id, gfxEntity.vecGfxObject.back() });
            gfxEntity.mapGfxObjectTreeNode.insert({ gfxEntity.vecGfxObject.back(), id });
        }
    });

    for (const GraphicsObjectPtr& gfxObject : gfxEntity.vecGfxObject) {
        m_gfxScene.addObject(gfxObject);
        auto driver = GraphicsObjectDriver::get(gfxObject);
        if (driver)
            driver->applyDisplayMode(gfxObject, this->activeDisplayMode(driver));
    }

    m_gfxScene.redraw();

    traverseTree(entityTreeNodeId, docModelTree, [=](TreeNodeId id) {
        m_mapTreeNodeCheckState.insert({ id, Qt::Checked });
    });

    GraphicsUtils::V3dView_fitAll(m_v3dView);
    for (const GraphicsObjectPtr& gfxObject : gfxEntity.vecGfxObject) {
        const Bnd_Box bndBox = GraphicsUtils::AisObject_boundingBox(gfxObject);
        BndUtils::add(&m_gfxBoundingBox, bndBox);
    }

    m_vecGraphicsEntity.push_back(std::move(gfxEntity));
}

const GuiDocument::GraphicsEntity* GuiDocument::findGraphicsEntity(TreeNodeId entityTreeNodeId) const
{
    auto itFound = std::find_if(
                m_vecGraphicsEntity.cbegin(),
                m_vecGraphicsEntity.cend(),
                [=](const GraphicsEntity& item) { return item.treeNodeId == entityTreeNodeId; });
    return itFound != m_vecGraphicsEntity.cend() ? &(*itFound) : nullptr;
}

void GuiDocument::v3dViewTrihedronDisplay(Qt::Corner corner)
{
    constexpr double scale = 0.075;
    m_v3dView->TriedronDisplay(toOccCorner(corner), Quantity_NOC_GRAY50, scale, V3d_ZBUFFER);
}

} // namespace Mayo
