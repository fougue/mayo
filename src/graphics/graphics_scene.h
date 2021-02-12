/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
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
    ~GraphicsScene();

    opencascade::handle<V3d_View> createV3dView();

    const opencascade::handle<V3d_Viewer>& v3dViewer() const;
    const opencascade::handle<Prs3d_Drawer>& defaultPrs3dDrawer() const;
    const opencascade::handle<StdSelect_ViewerSelector3d>& mainSelector() const;
    bool hiddenLineDrawingOn() const;

    void addObject(const GraphicsObjectPtr& object);
    void eraseObject(const GraphicsObjectPtr& object);

    void redraw();
    bool isRedrawBlocked() const;
    void blockRedraw(bool on);

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

    enum class SelectionMode {
        None, Single, Multi
    };
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);

    const GraphicsOwnerPtr& currentHighlightedOwner() const;
    void highlightAt(const QPoint& pos, const Handle_V3d_View& view);
    void select();

    int selectedCount() const;

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
    void selectionChanged();

private:
    AIS_InteractiveContext* aisContextPtr() const;

    class Private;
    Private* const d;
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
    this->aisContextPtr()->DisplayedObjects(listObject);
    for (const GraphicsObjectPtr& ptr : listObject)
        fn(ptr);
}

template<typename FUNCTION>
void GraphicsScene::foreachOwner(const GraphicsObjectPtr& object, int selectionMode, FUNCTION fn) const
{
    opencascade::handle<SelectMgr_IndexedMapOfOwner> mapEntityOwner;
    this->aisContextPtr()->EntityOwners(mapEntityOwner, object, selectionMode);
    for (auto it = mapEntityOwner->cbegin(); it != mapEntityOwner->cend(); ++it)
        fn(*it);
}

template<typename FUNCTION>
void GraphicsScene::foreachSelectedOwner(FUNCTION fn) const
{
    auto context = this->aisContextPtr();
    context->InitSelected();
    while (context->MoreSelected()) {
        fn(context->SelectedOwner());
        context->NextSelected();
    }
}

template<typename PREDICATE>
GraphicsOwnerPtr GraphicsScene::findSelectedOwner(PREDICATE fn) const
{
    auto context = this->aisContextPtr();
    context->InitSelected();
    while (context->MoreSelected()) {
        if (fn(context->SelectedOwner()))
            return context->SelectedOwner();

        context->NextSelected();
    }

    return GraphicsOwnerPtr();
}

} // namespace Mayo
