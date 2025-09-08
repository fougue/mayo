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
#include <V3d_AmbientLight.hxx>
#include <V3d_DirectionalLight.hxx>

#include <unordered_set>

namespace Mayo {

// Defined in graphics_create_driver.cpp
OccHandle<Graphic3d_GraphicDriver> graphicsCreateDriver();

namespace Internal {

static OccHandle<V3d_Viewer> createOccViewer()
{
    auto viewer = makeOccHandle<V3d_Viewer>(graphicsCreateDriver());
    viewer->SetDefaultViewSize(1000.);
    viewer->SetDefaultViewProj(V3d_XposYnegZpos);
    viewer->SetComputedMode(true);
    viewer->SetDefaultComputedMode(true);
//    viewer->SetDefaultVisualization(V3d_ZBUFFER);
//    viewer->SetDefaultShadingModel(V3d_GOURAUD);
    viewer->SetDefaultLights();

//    auto ambientLight = new V3d_AmbientLight;
//    ambientLight->SetIntensity(0.7f);
//    viewer->AddLight(ambientLight);

//    auto dirLight = new V3d_DirectionalLight;
//    dirLight->SetIntensity(0.8f);
//    dirLight->SetHeadlight(true);
//    dirLight->SetDirection(0, 0, -1);
//    dirLight->SetSmoothAngle(3.14159265f * 6.f / 180.f);
//    viewer->AddLight(dirLight);

    viewer->SetLightOn();

#if 0
    for (const OccHandle<Graphic3d_CLight>& light : viewer->DefinedLights()) {
        if (light->Name() == "amblight" || light->Type() == Graphic3d_TypeOfLightSource_Ambient) {
            light->SetIntensity(1.f);
        }
        else if (light->Name() == "headlight" || light->Type() == Graphic3d_TypeOfLightSource_Directional) {
            light->SetIntensity(5.f);
            light->SetHeadlight(true);
            light->SetDirection(0, 0, -1);
        }
    }
#endif

    return viewer;
}

} // namespace Internal

namespace {

class InteractiveContext : public AIS_InteractiveContext {
    DEFINE_STANDARD_RTTI_INLINE(InteractiveContext, AIS_InteractiveContext)
public:
    InteractiveContext(const OccHandle<V3d_Viewer>& viewer)
        : AIS_InteractiveContext(viewer)
    {}

    constexpr const GraphicsOwnerPtr& member_myLastPicked() const { return myLastPicked; }

    int defaultDisplayMode(const GraphicsObjectPtr& object) const
    {
        int displayMode;
        int hilightMode;
        int selMode;
        this->GetDefModes(object, displayMode, hilightMode, selMode);
        return displayMode;
    }
};

DEFINE_STANDARD_HANDLE(InteractiveContext, AIS_InteractiveContext)

} // namespace

class GraphicsScene::Private {
public:
    OccHandle<InteractiveContext> m_aisContext;
    std::unordered_set<const AIS_InteractiveObject*> m_setClipPlaneSensitive;
    bool m_isRedrawBlocked = false;
    SelectionMode m_selectionMode = SelectionMode::Single;
};

GraphicsScene::GraphicsScene()
    : d(new Private)
{
    d->m_aisContext = new InteractiveContext(Internal::createOccViewer());
}

GraphicsScene::~GraphicsScene()
{
    // Preventive cleaning fixes weird crash happening in MSVC Debug mode
    d->m_aisContext->RemoveFilters();
    d->m_aisContext->Deactivate();
    d->m_aisContext->EraseAll(false);
    d->m_aisContext->RemoveAll(false);
    delete d;
}

OccHandle<V3d_View> GraphicsScene::createV3dView()
{
    return this->v3dViewer()->CreateView();
}

const OccHandle<V3d_Viewer>& GraphicsScene::v3dViewer() const
{
    return d->m_aisContext->CurrentViewer();
}

const OccHandle<StdSelect_ViewerSelector3d>& GraphicsScene::mainSelector() const
{
    return d->m_aisContext->MainSelector();
}

bool GraphicsScene::hiddenLineDrawingOn() const
{
    return d->m_aisContext->DrawHiddenLine();
}

const OccHandle<Prs3d_Drawer>& GraphicsScene::drawerDefault() const
{
    return d->m_aisContext->DefaultDrawer();
}

const OccHandle<Prs3d_Drawer>& GraphicsScene::drawerHighlight(Prs3d_TypeOfHighlight style) const
{
    return d->m_aisContext->HighlightStyle(style);
}

void GraphicsScene::addObject(const GraphicsObjectPtr& object, AddObjectFlags flags)
{
    if (object) {
        if ((flags & AddObjectDisableSelectionMode) != 0) {
            const bool onEntry_AutoActivateSelection = d->m_aisContext->GetAutoActivateSelection();
            const int defaultDisplayMode = d->m_aisContext->defaultDisplayMode(object);
            d->m_aisContext->SetAutoActivateSelection(false);
            d->m_aisContext->Display(object, defaultDisplayMode, -1, false);
            d->m_aisContext->SetAutoActivateSelection(onEntry_AutoActivateSelection);
        }
        else {
            d->m_aisContext->Display(object, false);
        }
    }
}

void GraphicsScene::eraseObject(const GraphicsObjectPtr& object)
{
    GraphicsUtils::AisContext_eraseObject(d->m_aisContext, object);
    d->m_setClipPlaneSensitive.erase(object.get());
}

void GraphicsScene::redraw()
{
    if (d->m_isRedrawBlocked)
        return;

    //d->m_aisContext->UpdateCurrentViewer();
    for (auto itView = this->v3dViewer()->DefinedViewIterator(); itView.More(); itView.Next())
        this->signalRedrawRequested.send(itView.Value());
}

void GraphicsScene::redraw(const OccHandle<V3d_View>& view)
{
    if (d->m_isRedrawBlocked)
        return;

    for (auto itView = this->v3dViewer()->DefinedViewIterator(); itView.More(); itView.Next()) {
        if (itView.Value() == view) {
            this->signalRedrawRequested.send(view);
            break;
        }
    }
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

void GraphicsScene::deactivateObjectSelection(const GraphicsObjectPtr &object)
{
    d->m_aisContext->Deactivate(object);
}

void GraphicsScene::addSelectionFilter(const OccHandle<SelectMgr_Filter>& filter)
{
    d->m_aisContext->AddFilter(filter);
}

void GraphicsScene::removeSelectionFilter(const OccHandle<SelectMgr_Filter>& filter)
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

gp_Trsf GraphicsScene::objectTransformation(const GraphicsObjectPtr& object) const
{
    return d->m_aisContext->Location(object);
}

void GraphicsScene::setObjectTransformation(const GraphicsObjectPtr &object, const gp_Trsf &trsf)
{
    d->m_aisContext->SetLocation(object, trsf);
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
    const bool onEntryOwnerSelected = !this->firstSelectedOwner().IsNull();
    d->m_aisContext->ClearDetected(false);
    d->m_aisContext->ClearSelected(false);
    if (onEntryOwnerSelected)
        this->signalSelectionChanged.send();
}

AIS_InteractiveContext* GraphicsScene::aisContextPtr() const
{
    return d->m_aisContext.get();
}

void GraphicsScene::setOwnerSelected(const GraphicsOwnerPtr& owner, bool on)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    d->m_aisContext->SetSelectedState(owner, on);
#else
    // NOTE
    // This is not a perfect replacement of AIS_InteractiveContext:SetSelectedState() which first
    // appeared in OCC 7.4
    // This might create some bugs with old OCC versions(support of OCC < 7.4 will be dropped soon
    // so that's acceptable)
    d->m_aisContext->AddOrRemoveSelected(owner, false);
#endif
}

void GraphicsScene::toggleOwnerSelected(const GraphicsOwnerPtr& gfxOwner)
{
    auto gfxObject = GraphicsObjectPtr::DownCast(
        gfxOwner ? gfxOwner->Selectable() : OccHandle<SelectMgr_SelectableObject>()
    );
    if (GraphicsUtils::AisObject_isVisible(gfxObject))
        d->m_aisContext->AddOrRemoveSelected(gfxOwner, false);
}

void GraphicsScene::highlightAt(int xPos, int yPos, const OccHandle<V3d_View>& view)
{
    d->m_aisContext->MoveTo(xPos, yPos, view, false);
}

void GraphicsScene::select()
{
    if (d->m_selectionMode == SelectionMode::None)
        return;

    if (d->m_selectionMode == SelectionMode::Single) {
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        d->m_aisContext->SelectDetected(AIS_SelectionScheme_Replace);
#else
        d->m_aisContext->Select(false);
#endif
    }
    else if (d->m_selectionMode == SelectionMode::Multi) {
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        d->m_aisContext->SelectDetected(AIS_SelectionScheme_XOR);
#else
        d->m_aisContext->ShiftSelect(false);
#endif
    }

    this->signalSelectionChanged.send();
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

GraphicsScene::SelectionMode GraphicsScene::selectionMode() const
{
    return d->m_selectionMode;
}

void GraphicsScene::setSelectionMode(GraphicsScene::SelectionMode mode)
{
    if (mode != d->m_selectionMode) {
        d->m_selectionMode = mode;
        this->signalSelectionModeChanged.send();
    }
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
