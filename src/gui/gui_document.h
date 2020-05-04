/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../graphics/graphics_entity.h"

#include <QtCore/QObject>
#include <AIS_InteractiveContext.hxx>
#include <Bnd_Box.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <memory>
#include <vector>
class TopoDS_Face;

namespace Mayo {

class ApplicationItem;

class GuiDocument : public QObject {
    Q_OBJECT
public:
    GuiDocument(const DocumentPtr& doc);

    const DocumentPtr& document() const;
    const Handle_V3d_View& v3dView() const;
    const Handle_AIS_InteractiveContext& aisInteractiveContext() const;
    //GpxDocumentItem* findItemGpx(const DocumentItem* item) const;

    const Bnd_Box& gpxBoundingBox() const;

    std::vector<Handle_SelectMgr_EntityOwner> selectedEntityOwners() const;
    void toggleItemSelected(const ApplicationItem& appItem);
    void clearItemSelection();

    bool isOriginTrihedronVisible() const;
    void toggleOriginTrihedronVisibility();

    void updateV3dViewer();

signals:
    void gpxBoundingBoxChanged(const Bnd_Box& bndBox);

private:
    void onDocumentEntityAdded(TreeNodeId entityTreeNodeId);
    void onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId);

    void mapGraphics(TreeNodeId entityTreeNodeId);

    using ArrayGraphicsEntityOwner = std::vector<Handle_SelectMgr_EntityOwner>;
    struct GraphicsItem {
        GraphicsEntity graphicsEntity;
        TreeNodeId entityTreeNodeId;
        ArrayGraphicsEntityOwner vecGpxEntityOwner;
        Handle_SelectMgr_EntityOwner findBrepOwner(const TopoDS_Face& face) const;
    };

    DocumentPtr m_document;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveContext m_aisContext;
    Handle_AIS_InteractiveObject m_aisOriginTrihedron;
    std::vector<GraphicsItem> m_vecGraphicsItem;
    Bnd_Box m_gpxBoundingBox;
};

} // namespace Mayo
