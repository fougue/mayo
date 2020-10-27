/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_ptr.h"
#include "graphics_owner_ptr.h"

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <QtCore/QObject>
#include <unordered_set>
class QPoint;

namespace Mayo {

// Provides a container for GraphicsObject items(actually AIS_InteractiveObject)
// It's a wrapper(incomplete though) around AIS_InteractiveContext to provide a more consistent API
class GraphicsScene : public QObject {
    Q_OBJECT
public:
    GraphicsScene(QObject* parent = nullptr);

    opencascade::handle<V3d_View> createV3dView();

    const opencascade::handle<V3d_Viewer>& v3dViewer() const { return m_v3dViewer; }
    const opencascade::handle<Prs3d_Drawer>& defaultPrs3dDrawer() const { return m_aisContext->DefaultDrawer(); }
    const opencascade::handle<StdSelect_ViewerSelector3d>& mainSelector() const { return m_aisContext->MainSelector(); }
    bool hiddenLineDrawingOn() const { return m_aisContext->DrawHiddenLine(); }

    void addObject(const GraphicsObjectPtr& object);
    void eraseObject(const GraphicsObjectPtr& object);

    void redraw();
    bool isRedrawBlocked() const { return m_isRedrawBlocked; }
    void blockRedraw(bool on) { m_isRedrawBlocked = on; }

    void recomputeObjectPresentation(const GraphicsObjectPtr& object);

    void activateObjectSelection(const GraphicsObjectPtr& object, int mode);
    void deactivateObjectSelection(const GraphicsObjectPtr& object, int mode);

    void addSelectionFilter(const Handle_SelectMgr_Filter& filter);
    void removeSelectionFilter(const Handle_SelectMgr_Filter& filter);
    void clearSelectionFilters();

    void setObjectDisplayMode(const GraphicsObjectPtr& object, int displayMode);

    bool isObjectClipPlaneSensitive(const GraphicsObjectPtr& object) const;
    void setObjectClipPlaneSensitive(const GraphicsObjectPtr& object, bool on);

    bool isObjectVisible(const GraphicsObjectPtr& object) const;
    void setObjectVisible(const GraphicsObjectPtr& object, bool on);

    void highlightAt(const QPoint& pos, const Handle_V3d_View& view);
    void selectCurrentHighlighted();

    const GraphicsOwnerPtr& currentHighlightedOwner() const { return m_aisContext->DetectedOwner(); }

    GraphicsOwnerPtr firstSelectedOwner() const;
    void toggleOwnerSelection(const GraphicsOwnerPtr& owner);
    void clearSelection();

    template<typename FUNCTION>
    void foreachDisplayedObject(FUNCTION fn) const;

    template<typename FUNCTION>
    void foreachOwner(const GraphicsObjectPtr& object, int selectionMode, FUNCTION fn) const;

    template<typename FUNCTION>
    void foreachSelectedOwner(FUNCTION fn) const;

    template<typename PREDICATE>
    GraphicsOwnerPtr findSelectedOwner(PREDICATE fn) const;

signals:
    void selectionCleared();
    void singleItemSelected();

private:
    Handle_V3d_Viewer m_v3dViewer;
    Handle_AIS_InteractiveContext m_aisContext;
    std::unordered_set<const AIS_InteractiveObject*> m_setClipPlaneSensitive;
    bool m_isRedrawBlocked = false;
};

class GraphicsSceneRedrawBlocker {
public:
    GraphicsSceneRedrawBlocker(GraphicsScene* scene);
    ~GraphicsSceneRedrawBlocker();

    GraphicsSceneRedrawBlocker(const GraphicsSceneRedrawBlocker&) = delete;
    GraphicsSceneRedrawBlocker(GraphicsSceneRedrawBlocker&&) = delete;
    GraphicsSceneRedrawBlocker& operator=(const GraphicsSceneRedrawBlocker&) = delete;
    GraphicsSceneRedrawBlocker& operator=(GraphicsSceneRedrawBlocker&&) = delete;

private:
    GraphicsScene* m_scene = nullptr;
    bool m_isRedrawBlockedOnEntry = false;
};




// --
// -- Implementation
// --

template<typename FUNCTION>
void GraphicsScene::foreachDisplayedObject(FUNCTION fn) const
{
    AIS_ListOfInteractive listObject;
    m_aisContext->DisplayedObjects(listObject);
    for (const GraphicsObjectPtr& ptr : listObject)
        fn(ptr);
}

template<typename FUNCTION>
void GraphicsScene::foreachOwner(const GraphicsObjectPtr& object, int selectionMode, FUNCTION fn) const
{
    opencascade::handle<SelectMgr_IndexedMapOfOwner> mapEntityOwner;
    m_aisContext->EntityOwners(mapEntityOwner, object, selectionMode);
    for (auto it = mapEntityOwner->cbegin(); it != mapEntityOwner->cend(); ++it)
        fn(*it);
}

template<typename FUNCTION>
void GraphicsScene::foreachSelectedOwner(FUNCTION fn) const
{
    m_aisContext->InitSelected();
    while (m_aisContext->MoreSelected()) {
        fn(m_aisContext->SelectedOwner());
        m_aisContext->NextSelected();
    }
}

template<typename PREDICATE>
GraphicsOwnerPtr GraphicsScene::findSelectedOwner(PREDICATE fn) const
{
    m_aisContext->InitSelected();
    while (m_aisContext->MoreSelected()) {
        if (fn(m_aisContext->SelectedOwner()))
            return m_aisContext->SelectedOwner();

        m_aisContext->NextSelected();
    }

    return GraphicsOwnerPtr();
}

} // namespace Mayo
