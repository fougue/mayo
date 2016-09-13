#pragma once

#include <QtCore/QObject>
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <vector>

namespace Mayo {

class Document;
class DocumentItem;
class GpxDocumentItem;
class WidgetGuiDocumentView3d;

class GuiDocument : public QObject
{
public:
    GuiDocument(Document* doc);

    Document* document() const;
    WidgetGuiDocumentView3d* widgetView3d() const;
    GpxDocumentItem* findItemGpx(const DocumentItem* item) const;

private:
    void onItemAdded(DocumentItem* item);
    void onItemErased(const DocumentItem* item);

    struct DocumentItem_Gpx {
        DocumentItem* item;
        GpxDocumentItem* gpx;
    };

    Document* m_document = nullptr;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_AIS_InteractiveContext m_aisContext;
    WidgetGuiDocumentView3d* m_guiDocView3d = nullptr;
    std::vector<DocumentItem_Gpx> m_vecDocItemGpx;
};

} // namespace Mayo
