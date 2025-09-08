/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"
#include "../base/signal.h"
#include "graphics_object_ptr.h"
#include "graphics_owner_ptr.h"

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>

namespace Mayo {

// Provides a container for GraphicsObject items(actually AIS_InteractiveObject)
// It's a wrapper(incomplete though) around AIS_InteractiveContext to provide a more consistent API
class GraphicsScene {
public:
    GraphicsScene();
    ~GraphicsScene();

    // Not copyable
    GraphicsScene(const GraphicsScene&) = delete;
    GraphicsScene& operator=(const GraphicsScene&) = delete;

    OccHandle<V3d_View> createV3dView();

    const OccHandle<V3d_Viewer>& v3dViewer() const;
    const OccHandle<StdSelect_ViewerSelector3d>& mainSelector() const;
    bool hiddenLineDrawingOn() const;

    const OccHandle<Prs3d_Drawer>& drawerDefault() const;
    const OccHandle<Prs3d_Drawer>& drawerHighlight(Prs3d_TypeOfHighlight style) const;

    enum AddObjectFlag {
        AddObjectDefault = 0,
        AddObjectDisableSelectionMode = 1
    };
    using AddObjectFlags = unsigned;

    void addObject(const GraphicsObjectPtr& object, AddObjectFlags flags = AddObjectDefault);
    void eraseObject(const GraphicsObjectPtr& object);

    void redraw();
    void redraw(const OccHandle<V3d_View>& view);
    bool isRedrawBlocked() const;
    void blockRedraw(bool on);

    void recomputeObjectPresentation(const GraphicsObjectPtr& object);

    void activateObjectSelection(const GraphicsObjectPtr& object, int mode);
    void deactivateObjectSelection(const GraphicsObjectPtr& object, int mode);
    void deactivateObjectSelection(const GraphicsObjectPtr& object);

    void addSelectionFilter(const OccHandle<SelectMgr_Filter>& filter);
    void removeSelectionFilter(const OccHandle<SelectMgr_Filter>& filter);
    void clearSelectionFilters();

    void setObjectDisplayMode(const GraphicsObjectPtr& object, int displayMode);

    bool isObjectClipPlaneSensitive(const GraphicsObjectPtr& object) const;
    void setObjectClipPlaneSensitive(const GraphicsObjectPtr& object, bool on);

    bool isObjectVisible(const GraphicsObjectPtr& object) const;
    void setObjectVisible(const GraphicsObjectPtr& object, bool on);

    gp_Trsf objectTransformation(const GraphicsObjectPtr& object) const;
    void setObjectTransformation(const GraphicsObjectPtr& object, const gp_Trsf& trsf);

    enum class SelectionMode {
        None, Single, Multi
    };
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);

    const GraphicsOwnerPtr& currentHighlightedOwner() const;
    void highlightAt(int xPos, int yPos, const OccHandle<V3d_View>& view);
    void select();

    int selectedCount() const;

    GraphicsOwnerPtr firstSelectedOwner() const;
    void setOwnerSelected(const GraphicsOwnerPtr& owner, bool on);
    void toggleOwnerSelected(const GraphicsOwnerPtr& owner);
    void clearSelection();

    template<typename Function>
    void foreachDisplayedObject(Function fn) const;

    template<typename Function>
    void foreachActiveSelectionMode(const GraphicsObjectPtr& object, Function fn) const;

    template<typename Function>
    void foreachOwner(const GraphicsObjectPtr& object, int selectionMode, Function fn) const;

    template<typename Function>
    void foreachSelectedOwner(Function fn) const;

    template<typename Predicate>
    GraphicsOwnerPtr findSelectedOwner(Predicate fn) const;

    // Signals
    Signal<> signalSelectionChanged;
    Signal<> signalSelectionModeChanged;
    Signal<const OccHandle<V3d_View>&> signalRedrawRequested;

private:
    AIS_InteractiveContext* aisContextPtr() const;

    class Private;
    Private* const d = nullptr;
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

template<typename Function>
void GraphicsScene::foreachDisplayedObject(Function fn) const
{
    AIS_ListOfInteractive listObject;
    this->aisContextPtr()->DisplayedObjects(listObject);
    for (const GraphicsObjectPtr& ptr : listObject)
        fn(ptr);
}

template<typename Function>
void GraphicsScene::foreachActiveSelectionMode(const GraphicsObjectPtr& object, Function fn) const
{
    TColStd_ListOfInteger listMode;
    this->aisContextPtr()->ActivatedModes(object, listMode);
    for (GraphicsObjectSelectionMode mode : listMode)
        fn(mode);
}

template<typename Function>
void GraphicsScene::foreachOwner(const GraphicsObjectPtr& object, int selectionMode, Function fn) const
{
    OccHandle<SelectMgr_IndexedMapOfOwner> mapEntityOwner;
    this->aisContextPtr()->EntityOwners(mapEntityOwner, object, selectionMode);
    for (auto it = mapEntityOwner->cbegin(); it != mapEntityOwner->cend(); ++it)
        fn(*it);
}

template<typename Function>
void GraphicsScene::foreachSelectedOwner(Function fn) const
{
    auto context = this->aisContextPtr();
    for (context->InitSelected(); context->MoreSelected(); context->NextSelected()) {
        fn(context->SelectedOwner());
    }
}

template<typename Predicate>
GraphicsOwnerPtr GraphicsScene::findSelectedOwner(Predicate fn) const
{
    auto context = this->aisContextPtr();
    for (context->InitSelected(); context->MoreSelected(); context->NextSelected()) {
        if (fn(context->SelectedOwner()))
            return context->SelectedOwner();
    }

    return {};
}

} // namespace Mayo
