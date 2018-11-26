/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <AIS_InteractiveContext.hxx>
#include <Bnd_Box.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <vector>
class TopoDS_Face;

namespace Mayo {

class ApplicationItem;
class Document;
class DocumentItem;
class GpxDocumentItem;

class GuiDocument : public QObject {
    Q_OBJECT
public:
    GuiDocument(Document* doc);

    Document* document() const;
    const Handle_V3d_View& v3dView() const;
    const Handle_AIS_InteractiveContext& aisInteractiveContext() const;
    GpxDocumentItem* findItemGpx(const DocumentItem* item) const;

    const Bnd_Box& gpxBoundingBox() const;

    std::vector<Handle_SelectMgr_EntityOwner> selectedEntityOwners() const;
    void toggleItemSelected(const ApplicationItem& appItem);
    void clearItemSelection();

    bool isOriginTrihedronVisible() const;
    void toggleOriginTrihedronVisibility();

    void updateV3dViewer();

    bool isDetectionEnabled() const;
    void setDetectionEnabled(bool on);

    gp_Pnt toPoint3d(double x, double y) const;

signals:
    void gpxBoundingBoxChanged(const Bnd_Box& bndBox);

private:
    void onItemAdded(DocumentItem* item);
    void onItemErased(const DocumentItem* item);

    using ArrayGpxEntityOwner = std::vector<Handle_SelectMgr_EntityOwner>;
    struct GuiDocumentItem {
        GuiDocumentItem() = default;
        GuiDocumentItem(DocumentItem* item, GpxDocumentItem* gpx);
        DocumentItem* docItem;
        GpxDocumentItem* gpxDocItem;
        ArrayGpxEntityOwner vecGpxEntityOwner;
        Handle_SelectMgr_EntityOwner findBrepOwner(const TopoDS_Face& face) const;
    };
    const GuiDocumentItem* findGuiDocumentItem(const DocumentItem* item) const;

    Document* m_document = nullptr;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveContext m_aisContext;
    Handle_AIS_InteractiveObject m_aisOriginTrihedron;
    std::vector<GuiDocumentItem> m_vecGuiDocumentItem;
    Bnd_Box m_gpxBoundingBox;
    bool m_isDetectionEnabled = false;
};

} // namespace Mayo
