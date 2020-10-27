/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_scene.h"

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
//    viewer->SetDefaultComputedMode(true);
//    viewer->SetDefaultVisualization(V3d_ZBUFFER);
//    viewer->SetDefaultShadingModel(V3d_GOURAUD);
    viewer->SetDefaultLights();
    viewer->SetLightOn();
    return viewer;
}

} // namespace Internal

GraphicsScene::GraphicsScene(QObject* parent)
    : QObject(parent),
      m_v3dViewer(Mayo::Internal::createOccViewer()),
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer))
{
}

opencascade::handle<V3d_View> GraphicsScene::createV3dView()
{
    return m_v3dViewer->CreateView();
}

void GraphicsScene::addObject(const GraphicsObjectPtr& object)
{
    m_aisContext->Display(object, false);
}

void GraphicsScene::eraseObject(const GraphicsObjectPtr& object)
{
    Mayo::GraphicsUtils::AisContext_eraseObject(m_aisContext, object);
    m_setClipPlaneSensitive.erase(object.get());
}

void GraphicsScene::redraw()
{
    if (!m_isRedrawBlocked)
        m_aisContext->UpdateCurrentViewer();
}

void GraphicsScene::recomputeObjectPresentation(const GraphicsObjectPtr& object)
{
    m_aisContext->Redisplay(object, false);
}

void GraphicsScene::activateObjectSelection(const GraphicsObjectPtr& object, int mode)
{
    m_aisContext->Activate(object, mode);
}

void GraphicsScene::deactivateObjectSelection(const Mayo::GraphicsObjectPtr &object, int mode)
{
    m_aisContext->Deactivate(object, mode);
}

void GraphicsScene::addSelectionFilter(const Handle_SelectMgr_Filter& filter)
{
    m_aisContext->AddFilter(filter);
}

void GraphicsScene::removeSelectionFilter(const Handle_SelectMgr_Filter& filter)
{
    m_aisContext->RemoveFilter(filter);
}

void GraphicsScene::clearSelectionFilters()
{
    m_aisContext->RemoveFilters();
}

void GraphicsScene::setObjectDisplayMode(const GraphicsObjectPtr& object, int displayMode)
{
    m_aisContext->SetDisplayMode(object, displayMode, false);
}

bool GraphicsScene::isObjectClipPlaneSensitive(const GraphicsObjectPtr& object) const
{
    if (object.IsNull())
        return false;

    return m_setClipPlaneSensitive.find(object.get()) != m_setClipPlaneSensitive.cend();
}

void GraphicsScene::setObjectClipPlaneSensitive(const GraphicsObjectPtr& object, bool on)
{
    if (object.IsNull())
        return;

    if (on)
        m_setClipPlaneSensitive.insert(object.get());
    else
        m_setClipPlaneSensitive.erase(object.get());
}

bool GraphicsScene::isObjectVisible(const GraphicsObjectPtr& object) const
{
    return m_aisContext->IsDisplayed(object);
}

void GraphicsScene::setObjectVisible(const GraphicsObjectPtr& object, bool on)
{
    GraphicsUtils::AisContext_setObjectVisible(m_aisContext, object, on);
}

GraphicsOwnerPtr GraphicsScene::firstSelectedOwner() const
{
    m_aisContext->InitSelected();
    if (m_aisContext->MoreSelected())
        return m_aisContext->SelectedOwner();

    return {};
}

void GraphicsScene::clearSelection()
{
    m_aisContext->ClearDetected(false);
    m_aisContext->ClearSelected(false);
}

void GraphicsScene::toggleOwnerSelection(const GraphicsOwnerPtr& gfxOwner)
{
    m_aisContext->AddOrRemoveSelected(gfxOwner, false);
}

void GraphicsScene::highlightAt(const QPoint& pos, const Handle_V3d_View& view)
{
    m_aisContext->MoveTo(pos.x(), pos.y(), view, true);
}

void GraphicsScene::selectCurrentHighlighted()
{
    const AIS_StatusOfPick pick = m_aisContext->Select(true);
    if (pick == AIS_SOP_NothingSelected)
        emit this->selectionCleared();
    else if (pick == AIS_SOP_OneSelected)
        emit this->singleItemSelected();
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
