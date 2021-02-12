/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_scene.h"

#include "../base/tkernel_utils.h"
#include "graphics_utils.h"

#include <Graphic3d_GraphicDriver.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <QtCore/QPoint>

namespace Mayo {
namespace Internal {

// Defined in graphics_create_driver.cpp
Handle_Graphic3d_GraphicDriver createGfxDriver();

static Handle_V3d_Viewer createOccViewer()
{
    Handle_V3d_Viewer viewer = new V3d_Viewer(createGfxDriver());
    viewer->SetDefaultViewSize(1000.);
    viewer->SetDefaultViewProj(V3d_XposYnegZpos);
    viewer->SetComputedMode(true);
    viewer->SetDefaultComputedMode(true);
//    viewer->SetDefaultVisualization(V3d_ZBUFFER);
//    viewer->SetDefaultShadingModel(V3d_GOURAUD);
    viewer->SetDefaultLights();
    viewer->SetLightOn();
    return viewer;
}

} // namespace Internal

namespace {

class InteractiveContext : public AIS_InteractiveContext {
    DEFINE_STANDARD_RTTI_INLINE(InteractiveContext, AIS_InteractiveContext)
public:
    InteractiveContext(const Handle_V3d_Viewer& viewer)
        : AIS_InteractiveContext(viewer)
    {}

    constexpr const GraphicsOwnerPtr& member_myLastPicked() const { return myLastPicked; }
};

DEFINE_STANDARD_HANDLE(InteractiveContext, AIS_InteractiveContext)

} // namespace

class GraphicsScene::Private {
public:
    Handle_V3d_Viewer m_v3dViewer;
    Handle_InteractiveContext m_aisContext;
    std::unordered_set<const AIS_InteractiveObject*> m_setClipPlaneSensitive;
    bool m_isRedrawBlocked = false;
    SelectionMode m_selectionMode = SelectionMode::Single;
};

GraphicsScene::GraphicsScene(QObject* parent)
    : QObject(parent),
      d(new Private)
{
    d->m_v3dViewer = Internal::createOccViewer();
    d->m_aisContext = new InteractiveContext(d->m_v3dViewer);
}

GraphicsScene::~GraphicsScene()
{
    delete d;
}

opencascade::handle<V3d_View> GraphicsScene::createV3dView()
{
    return d->m_v3dViewer->CreateView();
}

const opencascade::handle<V3d_Viewer>& GraphicsScene::v3dViewer() const
{
    return d->m_v3dViewer;
}

const opencascade::handle<Prs3d_Drawer>& GraphicsScene::defaultPrs3dDrawer() const
{
    return d->m_aisContext->DefaultDrawer();
}

const opencascade::handle<StdSelect_ViewerSelector3d>& GraphicsScene::mainSelector() const
{
    return d->m_aisContext->MainSelector();
}

bool GraphicsScene::hiddenLineDrawingOn() const
{
    return d->m_aisContext->DrawHiddenLine();
}

void GraphicsScene::addObject(const GraphicsObjectPtr& object)
{
    if (object)
        d->m_aisContext->Display(object, false);
}

void GraphicsScene::eraseObject(const GraphicsObjectPtr& object)
{
    GraphicsUtils::AisContext_eraseObject(d->m_aisContext, object);
    d->m_setClipPlaneSensitive.erase(object.get());
}

void GraphicsScene::redraw()
{
    if (!d->m_isRedrawBlocked)
        d->m_aisContext->UpdateCurrentViewer();
}

bool GraphicsScene::isRedrawBlocked() const
{
    return d->m_isRedrawBlocked;
}

void GraphicsScene::blockRedraw(bool on)
{
    d->m_isRedrawBlocked = on;
}

void GraphicsScene::recomputeObjectPresentation(const GraphicsObjectPtr& object)
{
    d->m_aisContext->Redisplay(object, false);
}

void GraphicsScene::activateObjectSelection(const GraphicsObjectPtr& object, int mode)
{
    d->m_aisContext->Activate(object, mode);
}

void GraphicsScene::deactivateObjectSelection(const Mayo::GraphicsObjectPtr &object, int mode)
{
    d->m_aisContext->Deactivate(object, mode);
}

void GraphicsScene::addSelectionFilter(const Handle_SelectMgr_Filter& filter)
{
    d->m_aisContext->AddFilter(filter);
}

void GraphicsScene::removeSelectionFilter(const Handle_SelectMgr_Filter& filter)
{
    d->m_aisContext->RemoveFilter(filter);
}

void GraphicsScene::clearSelectionFilters()
{
    d->m_aisContext->RemoveFilters();
}

void GraphicsScene::setObjectDisplayMode(const GraphicsObjectPtr& object, int displayMode)
{
    d->m_aisContext->SetDisplayMode(object, displayMode, false);
}

bool GraphicsScene::isObjectClipPlaneSensitive(const GraphicsObjectPtr& object) const
{
    if (object.IsNull())
        return false;

    return d->m_setClipPlaneSensitive.find(object.get()) != d->m_setClipPlaneSensitive.cend();
}

void GraphicsScene::setObjectClipPlaneSensitive(const GraphicsObjectPtr& object, bool on)
{
    if (object.IsNull())
        return;

    if (on)
        d->m_setClipPlaneSensitive.insert(object.get());
    else
        d->m_setClipPlaneSensitive.erase(object.get());
}

bool GraphicsScene::isObjectVisible(const GraphicsObjectPtr& object) const
{
    return d->m_aisContext->IsDisplayed(object);
}

void GraphicsScene::setObjectVisible(const GraphicsObjectPtr& object, bool on)
{
    GraphicsUtils::AisContext_setObjectVisible(d->m_aisContext, object, on);
}

GraphicsOwnerPtr GraphicsScene::firstSelectedOwner() const
{
    d->m_aisContext->InitSelected();
    if (d->m_aisContext->MoreSelected())
        return d->m_aisContext->SelectedOwner();

    return {};
}

void GraphicsScene::clearSelection()
{
    d->m_aisContext->ClearDetected(false);
    d->m_aisContext->ClearSelected(false);
}

AIS_InteractiveContext* GraphicsScene::aisContextPtr() const
{
    return d->m_aisContext.get();
}

void GraphicsScene::toggleOwnerSelection(const GraphicsOwnerPtr& gfxOwner)
{
    auto gfxObject = GraphicsObjectPtr::DownCast(
                gfxOwner ? gfxOwner->Selectable() : Handle_SelectMgr_SelectableObject());
    if (GraphicsUtils::AisObject_isVisible(gfxObject))
        d->m_aisContext->AddOrRemoveSelected(gfxOwner, false);
}

void GraphicsScene::highlightAt(const QPoint& pos, const Handle_V3d_View& view)
{
    d->m_aisContext->MoveTo(pos.x(), pos.y(), view, true);
}

void GraphicsScene::select()
{
    if (d->m_selectionMode == SelectionMode::None)
        return;

    if (d->m_selectionMode == SelectionMode::Single)
        d->m_aisContext->Select(true);
    else if (d->m_selectionMode == SelectionMode::Multi)
        d->m_aisContext->ShiftSelect(true);

    emit this->selectionChanged();
}

int GraphicsScene::selectedCount() const
{
    return d->m_aisContext->NbSelected();
}

const GraphicsOwnerPtr& GraphicsScene::currentHighlightedOwner() const
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    return d->m_aisContext->DetectedOwner();
#else
    return d->m_aisContext->member_myLastPicked();
#endif
}

GraphicsScene::SelectionMode GraphicsScene::selectionMode() const {
    return d->m_selectionMode;
}

void GraphicsScene::setSelectionMode(GraphicsScene::SelectionMode mode) {
    d->m_selectionMode = mode;
}

GraphicsSceneRedrawBlocker::GraphicsSceneRedrawBlocker(GraphicsScene* scene)
    : m_scene(scene),
      m_isRedrawBlockedOnEntry(scene->isRedrawBlocked())
{
    scene->blockRedraw(true);
}

GraphicsSceneRedrawBlocker::~GraphicsSceneRedrawBlocker()
{
    m_scene->blockRedraw(m_isRedrawBlockedOnEntry);
}

} // namespace Mayo
