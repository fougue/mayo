#pragma once

#include <QtWidgets/QWidget>
#include <vector>

class QtVariantPropertyManager;
class QtVariantProperty;

namespace Mayo {

class DocumentItem;

class DocumentItemPropsView : public QWidget
{
public:
    DocumentItemPropsView(QWidget* parent = nullptr);
    ~DocumentItemPropsView();

    void editDocumentItems(const std::vector<DocumentItem*>& vecDocItem);

private:
    class Ui_DocumentItemPropsView* m_ui = nullptr;

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
