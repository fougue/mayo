#include "document_item_props_view.h"

#include "options.h"
#include "ui_document_item_props_view.h"

#include <QtVariantEditorFactory>
#include <QtVariantPropertyManager>

namespace Mayo {

DocumentItemPropsView::DocumentItemPropsView(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui_DocumentItemPropsView),
      m_varPropMgr(new QtVariantPropertyManager(this))
{
    m_ui->setupUi(this);

    // Create editable properties
    m_propAisShapeTransparency =
            m_varPropMgr->addProperty(QVariant::Int, tr("Transparency"));
    m_propAisShapeTransparency->setAttribute(QStringLiteral("minimum"), 0);
    m_propAisShapeTransparency->setAttribute(QLatin1String("maximum"), 100);
    m_propAisShapeTransparency->setAttribute(QLatin1String("singleStep"), 10);

    m_propAisShapeDisplayMode =
            m_varPropMgr->addProperty(
                QtVariantPropertyManager::enumTypeId(), tr("Display mode"));
    m_propAisShapeDisplayMode->setAttribute(
                QLatin1String("enumNames"),
                QStringList(tr("Wireframe")) << tr("Shaded"));
    m_propAisShapeDisplayMode->setValue(1);

    m_propAisShapeShowFaceBoundary =
            m_varPropMgr->addProperty(QVariant::Bool, tr("Show Face Boundary"));

    m_propMeshVsDisplayMode =
            m_varPropMgr->addProperty(
                QtVariantPropertyManager::enumTypeId(), tr("Display mode"));
    m_propMeshVsDisplayMode->setAttribute(
                QLatin1String("enumNames"),
                QStringList(tr("Wireframe")) << tr("Shaded") << tr("Shrink"));
    m_propMeshVsDisplayMode->setValue(1);

    m_propMeshVsShowEdges =
            m_varPropMgr->addProperty(QVariant::Bool, tr("Show Edges"));
    m_propMeshVsShowNodes =
            m_varPropMgr->addProperty(QVariant::Bool, tr("Show Nodes"));

    m_propColor = m_varPropMgr->addProperty(QVariant::Color, tr("Color"));

    m_propMaterial =
            m_varPropMgr->addProperty(
                QtVariantPropertyManager::enumTypeId(), tr("Material"));
    QStringList materialNames;
    for (const Options::Material& mat : Options::materials())
        materialNames.push_back(mat.name);
    m_propMaterial->setAttribute(QLatin1String("enumNames"), materialNames);

    auto variantEditorFactory = new QtVariantEditorFactory(this);
    m_ui->propsBrowser_DocumentItem->setFactoryForManager(
                m_varPropMgr, variantEditorFactory);
    m_ui->propsBrowser_DocumentItem->addProperty(m_propMaterial);
    m_ui->propsBrowser_DocumentItem->addProperty(m_propColor);
}

DocumentItemPropsView::~DocumentItemPropsView()
{
    delete m_ui;
}

void DocumentItemPropsView::editDocumentItems(
        const std::vector<DocumentItem*>& vecDocItem)
{
    if (vecDocItem.size() == 1) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        m_varPropMgr->clear();

    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
    }
}

} // namespace Mayo
