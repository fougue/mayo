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

namespace Mayo {

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

signals:
    void gpxBoundingBoxChanged(const Bnd_Box& bndBox);

private:
    void onItemAdded(DocumentItem* item);
    void onItemErased(const DocumentItem* item);

    struct DocumentItem_Gpx {
        DocumentItem* item;
        GpxDocumentItem* gpx;
    };

    Document* m_document = nullptr;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveContext m_aisContext;
    std::vector<DocumentItem_Gpx> m_vecDocItemGpx;
    Bnd_Box m_gpxBoundingBox;
};

} // namespace Mayo
