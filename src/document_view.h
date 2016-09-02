#pragma once

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <QtWidgets/QWidget>
#include <unordered_map>

class QTreeWidgetItem;
class QVariant;
class QtProperty;
class QtVariantPropertyManager;
class QtVariantProperty;

namespace Mayo {

class Document;
class DocumentItem;
class PartItem;
class QtOccView;

class DocumentView : public QWidget
{
    Q_OBJECT

public:
    DocumentView(Document* doc, QWidget* parent = nullptr);
    ~DocumentView();

    const Document* document() const;
    Document* document();

    const QtOccView* qtOccView() const;

private:
    void onItemAdded(DocumentItem* docItem);
    void onTreeWidgetDocumentSelectionChanged();
    void onQVariantPropertyValueChanged(
            QtProperty* property, const QVariant& value);
    void connectPropertyValueChangeSignals(bool on);

    struct Item_GpxObject
    {
        Item_GpxObject() = default;
        Item_GpxObject(
                const DocumentItem* pItem,
                const Handle_AIS_InteractiveObject& pGpxObject);
        const DocumentItem* item = nullptr;
        Handle_AIS_InteractiveObject gpxObject;
    };
    const Item_GpxObject* selectedDocumentItem() const;

    Document* m_document = nullptr;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_AIS_InteractiveContext m_aisContext;
    std::unordered_map<QTreeWidgetItem*, Item_GpxObject> m_mapTreeItemGpxObject;
    class Ui_DocumentView* m_ui = nullptr;

    QtVariantPropertyManager* m_varPropMgr = nullptr;
    QtVariantProperty* m_propAisShapeTransparency = nullptr;
    QtVariantProperty* m_propAisShapeDisplayMode = nullptr;
    QtVariantProperty* m_propAisShapeShowFaceBoundary = nullptr;
    QtVariantProperty* m_propMeshVsDisplayMode = nullptr;
    QtVariantProperty* m_propMeshVsShowEdges = nullptr;
    QtVariantProperty* m_propMeshVsShowNodes = nullptr;
    QtVariantProperty* m_propMaterial = nullptr;
    QtVariantProperty* m_propColor = nullptr;
};

} // namespace Mayo
