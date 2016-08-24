#include "document_view.h"

#include "document.h"
#include "brep_shape_item.h"
#include "stl_mesh_item.h"
#include "qt_occ_view.h"
#include "qt_occ_view_controller.h"
#include "ui_document_view.h"
#include "fougtools/qttools/gui/item_view_utils.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/occtools/qt_utils.h"

#include <Standard_Version.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#if OCC_VERSION_HEX <= 0x60701
#  include <Graphic3d.hxx>
#else
#  include <OpenGl_GraphicDriver.hxx>
#endif
#include <TCollection_ExtendedString.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <XSDRAWSTLVRML_DataSource.hxx>

#include <QtCore/QFileInfo>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtVariantEditorFactory>
#include <QtVariantPropertyManager>

namespace Mayo {

namespace Internal {

static Handle_V3d_Viewer createOccViewer()
{
    // Create the graphic driver
    Handle_Graphic3d_GraphicDriver gpxDriver;

    Handle_Aspect_DisplayConnection dispConnection;
#if (!defined(Q_OS_WIN32) && (!defined(Q_OS_MAC) || defined(MACOSX_USE_GLX)))
    dispConnection = new Aspect_DisplayConnection(std::getenv("DISPLAY"));
#endif

#if OCC_VERSION_HEX >= 0x060800
    if (dispConnection.IsNull())
        dispConnection = new Aspect_DisplayConnection;
    gpxDriver = new OpenGl_GraphicDriver(dispConnection);
#else
    gpxDriver = Graphic3d::InitGraphicDriver(dispConnection);
#endif

    // Create the named OCC 3d viewer
    Handle_V3d_Viewer viewer = new V3d_Viewer(
                gpxDriver, reinterpret_cast<const short*>("Viewer3d"));

    // Configure the OCC 3d viewer
    viewer->SetDefaultViewSize(1000.);
    viewer->SetDefaultViewProj(V3d_XposYnegZpos);
    viewer->SetDefaultBackgroundColor(Quantity_NOC_BLACK);
    viewer->SetDefaultVisualization(V3d_ZBUFFER);
    viewer->SetDefaultShadingModel(V3d_GOURAUD);
    viewer->SetUpdateMode(V3d_WAIT);
    viewer->SetDefaultSurfaceDetail(V3d_TEX_NONE);

    // Initialize the OCC 3d viewer
#if OCC_VERSION_HEX < 0x60700
    viewer->Init();
#endif
    viewer->SetDefaultLights();
    viewer->SetLightOn();

    return viewer;
}

static QToolButton* createViewBtn(
        QWidget* parent, const QString& imageFile, const QString& tooltip)
{
    auto btn = new QToolButton(parent);
    btn->setIcon(QIcon(QString(":/images/%1.png").arg(imageFile)));
    btn->setIconSize(QSize(16, 16));
    btn->setFixedSize(24, 24);
    btn->setToolTip(tooltip);
    return btn;
}

static void connectViewProjBtn(
        QToolButton* btn, QtOccView* view, V3d_TypeOfOrientation proj)
{
    QObject::connect(
                btn, &QAbstractButton::clicked,
                [=]{ view->v3dView()->SetProj(proj); });
}

struct Material
{
    Graphic3d_NameOfMaterial occMat;
    QString name;
};

static const Material materials[] = {
    { Graphic3d_NOM_BRASS,
      QApplication::translate("Mayo::DocumentView", "Brass") },
    { Graphic3d_NOM_BRONZE,
      QApplication::translate("Mayo::DocumentView", "Bronze") },
    { Graphic3d_NOM_COPPER,
      QApplication::translate("Mayo::DocumentView", "Copper") },
    { Graphic3d_NOM_GOLD,
      QApplication::translate("Mayo::DocumentView", "Gold") },
    { Graphic3d_NOM_PEWTER,
      QApplication::translate("Mayo::DocumentView", "Pewter") },
    { Graphic3d_NOM_PLASTER,
      QApplication::translate("Mayo::DocumentView", "Plaster") },
    { Graphic3d_NOM_PLASTIC,
      QApplication::translate("Mayo::DocumentView", "Plastic") },
    { Graphic3d_NOM_SILVER,
      QApplication::translate("Mayo::DocumentView", "Silver") },
    { Graphic3d_NOM_STEEL,
      QApplication::translate("Mayo::DocumentView", "Steel") },
    { Graphic3d_NOM_STONE,
      QApplication::translate("Mayo::DocumentView", "Stone") },
    { Graphic3d_NOM_SHINY_PLASTIC,
      QApplication::translate("Mayo::DocumentView", "Shiny plastic") },
    { Graphic3d_NOM_SATIN,
      QApplication::translate("Mayo::DocumentView", "Satin") },
    { Graphic3d_NOM_METALIZED,
      QApplication::translate("Mayo::DocumentView", "Metalized") },
    { Graphic3d_NOM_NEON_GNC,
      QApplication::translate("Mayo::DocumentView", "Neon gnc") },
    { Graphic3d_NOM_CHROME,
      QApplication::translate("Mayo::DocumentView", "Chrome") },
    { Graphic3d_NOM_ALUMINIUM,
      QApplication::translate("Mayo::DocumentView", "Aluminium") },
    { Graphic3d_NOM_OBSIDIAN,
      QApplication::translate("Mayo::DocumentView", "Obsidian") },
    { Graphic3d_NOM_NEON_PHC,
      QApplication::translate("Mayo::DocumentView", "Neon phc") },
    { Graphic3d_NOM_JADE,
      QApplication::translate("Mayo::DocumentView", "Jade") },
    { Graphic3d_NOM_DEFAULT,
      QApplication::translate("Mayo::DocumentView", "Default") }
};

} // namespace Internal

DocumentView::DocumentView(Document *doc, QWidget *parent)
    : QWidget(parent),
      m_document(doc),
      m_v3dViewer(Internal::createOccViewer()),
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer)),
      m_ui(new Ui_DocumentView),
      m_varPropMgr(new QtVariantPropertyManager(this))
{
    Q_ASSERT(doc != nullptr);

    m_ui->setupUi(this);
    m_ui->widget_View3d->setV3dViewer(m_v3dViewer);
    new QtOccViewController(m_ui->widget_View3d);

    auto btnFitAll =
            Internal::createViewBtn(m_ui->widget_Right, "fit_all", tr("Fit All"));
    auto btnViewIso =
            Internal::createViewBtn(m_ui->widget_Right, "view_axo", tr("Isometric"));
    auto btnViewBack =
            Internal::createViewBtn(m_ui->widget_Right, "view_back", tr("Back"));
    auto btnViewFront =
            Internal::createViewBtn(m_ui->widget_Right, "view_front", tr("Front"));
    auto btnViewLeft =
            Internal::createViewBtn(m_ui->widget_Right, "view_left", tr("Left"));
    auto btnViewRight =
            Internal::createViewBtn(m_ui->widget_Right, "view_right", tr("Right"));
    auto btnViewTop =
            Internal::createViewBtn(m_ui->widget_Right, "view_top", tr("Top"));
    auto btnViewBottom =
            Internal::createViewBtn(m_ui->widget_Right, "view_bottom", tr("Bottom"));
    btnFitAll->move(0, 0);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewIso, btnFitAll);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBack, btnViewIso);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewFront, btnViewBack);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewLeft, btnViewFront);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewRight, btnViewLeft);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewTop, btnViewRight);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBottom, btnViewTop);

    Internal::connectViewProjBtn(btnViewIso, m_ui->widget_View3d, V3d_XposYnegZpos);
    Internal::connectViewProjBtn(btnViewBack, m_ui->widget_View3d, V3d_Xneg);
    Internal::connectViewProjBtn(btnViewFront, m_ui->widget_View3d, V3d_Xpos);
    Internal::connectViewProjBtn(btnViewLeft, m_ui->widget_View3d, V3d_Ypos);
    Internal::connectViewProjBtn(btnViewRight, m_ui->widget_View3d, V3d_Yneg);
    Internal::connectViewProjBtn(btnViewTop, m_ui->widget_View3d, V3d_Zpos);
    Internal::connectViewProjBtn(btnViewBottom, m_ui->widget_View3d, V3d_Zneg);
    QObject::connect(
                btnFitAll, &QAbstractButton::clicked,
                m_ui->widget_View3d, &QtOccView::fitAll);
    QObject::connect(
                doc, &Document::partImported,
                this, &DocumentView::onPartImported);
    QObject::connect(
                m_ui->treeWidget_Document->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &DocumentView::onTreeWidgetDocumentSelectionChanged);

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
    for (const Internal::Material& mat : Internal::materials)
        materialNames.push_back(mat.name);
    m_propMaterial->setAttribute(QLatin1String("enumNames"), materialNames);

    auto variantEditorFactory = new QtVariantEditorFactory(this);
    m_ui->propBrowser_DocumentItem->setFactoryForManager(
                m_varPropMgr, variantEditorFactory);
    m_ui->propBrowser_DocumentItem->addProperty(m_propMaterial);
    m_ui->propBrowser_DocumentItem->addProperty(m_propColor);

    this->onTreeWidgetDocumentSelectionChanged();
}

DocumentView::~DocumentView()
{
    delete m_ui;
}

const Document *DocumentView::document() const
{
    return m_document;
}

Document *DocumentView::document()
{
    return m_document;
}

void DocumentView::onPartImported(const PartItem* partItem)
{
    if (partItem->isNull())
        return;

    static const Quantity_Color defaultPartColor(0.33, 0.33, 0.33, Quantity_TOC_RGB);
    Handle_AIS_InteractiveObject aisObject;
    if (sameType<BRepShapeItem>(partItem)) {
        auto brepShapeItem = static_cast<const BRepShapeItem*>(partItem);
        Handle_AIS_Shape aisShape = new AIS_Shape(brepShapeItem->brepShape());
        aisShape->SetMaterial(Graphic3d_NOM_PLASTIC);
        aisShape->SetDisplayMode(AIS_Shaded);
        aisShape->SetColor(defaultPartColor);
        aisShape->Attributes()->SetFaceBoundaryDraw(Standard_True);
        aisObject = aisShape;
    }
    else if (sameType<StlMeshItem>(partItem)) {
        auto stlMeshItem = static_cast<const StlMeshItem*>(partItem);
        Handle_MeshVS_Mesh meshVisu = new MeshVS_Mesh;
        Handle_XSDRAWSTLVRML_DataSource dataSource =
                new XSDRAWSTLVRML_DataSource(stlMeshItem->stlMesh());
        meshVisu->SetDataSource(dataSource);
        // meshVisu->AddBuilder(..., Standard_False); -> No selection
        meshVisu->AddBuilder(new MeshVS_MeshPrsBuilder(meshVisu), Standard_True);
        // MeshVS_DrawerAttribute
        meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, Standard_False);
        meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, Standard_False);
        meshVisu->GetDrawer()->SetMaterial(
                    MeshVS_DA_FrontMaterial,
                    Graphic3d_MaterialAspect(Graphic3d_NOM_PLASTIC));
        meshVisu->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, defaultPartColor);
        meshVisu->SetDisplayMode(MeshVS_DMF_Shading);
        // Wireframe as default hilight mode
        meshVisu->SetHilightMode(MeshVS_DMF_WireFrame);
        meshVisu->SetMeshSelMethod(MeshVS_MSM_PRECISE);
        aisObject = meshVisu;
    }

    if (!aisObject.IsNull()) {
        const QString label = QFileInfo(partItem->filePath()).fileName();
        auto treeItem = new QTreeWidgetItem;

        // Label
        treeItem->setText(0, !label.trimmed().isEmpty() ? label : tr("<unnamed>"));
        treeItem->setToolTip(0, partItem->filePath());

        m_mapTreeItemGpxObject.emplace(
                    treeItem, Item_GpxObject(partItem, aisObject));
        m_ui->treeWidget_Document->addTopLevelItem(treeItem);

        m_aisContext->Display(aisObject);

        if (m_document->rootDocumentItems().size() == 1)
            m_ui->widget_View3d->fitAll();
    }
}

void DocumentView::onTreeWidgetDocumentSelectionChanged()
{
    const DocumentView::Item_GpxObject* itemGpxObject = this->selectedDocumentItem();
    if (itemGpxObject != nullptr) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);

        this->connectPropertyValueChangeSignals(false);

        const Handle_AIS_InteractiveObject& gpxObject = itemGpxObject->gpxObject;

        QtTreePropertyBrowser* propBrowser = m_ui->propBrowser_DocumentItem;
        Quantity_Color partColor;
        Graphic3d_NameOfMaterial partMaterial;
        if (sameType<BRepShapeItem>(itemGpxObject->item)) {
            propBrowser->insertProperty(m_propAisShapeTransparency, nullptr);
            propBrowser->insertProperty(
                        m_propAisShapeDisplayMode, m_propAisShapeTransparency);
            propBrowser->insertProperty(
                        m_propAisShapeShowFaceBoundary, m_propAisShapeDisplayMode);
            propBrowser->removeProperty(m_propMeshVsDisplayMode);
            propBrowser->removeProperty(m_propMeshVsShowEdges);
            propBrowser->removeProperty(m_propMeshVsShowNodes);

            // Transparency
            m_propAisShapeTransparency->setValue(gpxObject->Transparency() * 100);
            // Display mode
            if (gpxObject->DisplayMode() == AIS_WireFrame)
                m_propAisShapeDisplayMode->setValue(0);
            else if (gpxObject->DisplayMode() == AIS_Shaded)
                m_propAisShapeDisplayMode->setValue(1);
            // Show face boundary
            m_propAisShapeShowFaceBoundary->setValue(
                        gpxObject->Attributes()->FaceBoundaryDraw() == Standard_True);
            // Material
            partMaterial = gpxObject->Material();
            // Color
            gpxObject->Color(partColor);
        }
        else if (sameType<StlMeshItem>(itemGpxObject->item)) {
            auto meshVisu = static_cast<const MeshVS_Mesh*>(gpxObject.operator->());
            propBrowser->removeProperty(m_propAisShapeTransparency);
            propBrowser->removeProperty(m_propAisShapeDisplayMode);
            propBrowser->removeProperty(m_propAisShapeShowFaceBoundary);
            propBrowser->insertProperty(m_propMeshVsDisplayMode, nullptr);
            propBrowser->insertProperty(m_propMeshVsShowEdges, m_propMeshVsDisplayMode);
            propBrowser->insertProperty(m_propMeshVsShowNodes, m_propMeshVsShowEdges);

            // Display mode
            if (gpxObject->DisplayMode() == MeshVS_DMF_WireFrame)
                m_propMeshVsDisplayMode->setValue(0);
            else if (gpxObject->DisplayMode() == MeshVS_DMF_Shading)
                m_propMeshVsDisplayMode->setValue(1);
            else if (gpxObject->DisplayMode() == MeshVS_DMF_Shrink)
                m_propMeshVsDisplayMode->setValue(2);
            // Show edges
            Standard_Boolean boolVal;
            meshVisu->GetDrawer()->GetBoolean(MeshVS_DA_ShowEdges, boolVal);
            m_propMeshVsShowEdges->setValue(boolVal == Standard_True);
            // Show nodes
            meshVisu->GetDrawer()->GetBoolean(MeshVS_DA_DisplayNodes, boolVal);
            m_propMeshVsShowNodes->setValue(boolVal == Standard_True);
            // Material
            Graphic3d_MaterialAspect materialAspect;
            meshVisu->GetDrawer()->GetMaterial(
                        MeshVS_DA_FrontMaterial, materialAspect);
            partMaterial = materialAspect.Name();
            // Color
            meshVisu->GetDrawer()->GetColor(MeshVS_DA_InteriorColor, partColor);
        }

        // Material
        auto itMat =
                std::find_if(
                    std::cbegin(Internal::materials),
                    std::cend(Internal::materials),
                    [=](const Internal::Material& mat) {
            return mat.occMat == partMaterial;
        } );
        if (itMat != std::cend(Internal::materials))
            m_propMaterial->setValue(itMat - std::cbegin(Internal::materials));

        // Color
        m_propColor->setValue(occ::QtUtils::toQColor(partColor));

        this->connectPropertyValueChangeSignals(true);
    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
        this->connectPropertyValueChangeSignals(false);
    }
}

void DocumentView::onQVariantPropertyValueChanged(
        QtProperty *property, const QVariant &value)
{
    const Item_GpxObject* itemGpxObject = this->selectedDocumentItem();
    if (itemGpxObject != nullptr) {
        Handle_AIS_InteractiveObject gpxObject = itemGpxObject->gpxObject;
        Handle_AIS_InteractiveContext cxt = gpxObject->GetContext();

        if (property == m_propAisShapeTransparency) {
            cxt->SetTransparency(gpxObject, value.toDouble() / 100.);
        }
        else if (property == m_propAisShapeDisplayMode) {
            const AIS_DisplayMode displayMode =
                    value.toInt() == 1 ? AIS_Shaded : AIS_WireFrame;
            cxt->SetDisplayMode(gpxObject, displayMode);
        }
        else if (property == m_propAisShapeShowFaceBoundary) {
            gpxObject->Attributes()->SetFaceBoundaryDraw(value.toBool());
            gpxObject->Redisplay(Standard_True); // All modes
            cxt->UpdateCurrentViewer();
        }
        else if (property == m_propMeshVsDisplayMode) {
            int displayMode = -1;
            switch (value.toInt()) {
            case 0: displayMode = MeshVS_DMF_WireFrame; break;
            case 1: displayMode = MeshVS_DMF_Shading; break;
            case 2: displayMode = MeshVS_DMF_Shrink; break;
            }
            cxt->SetDisplayMode(gpxObject, displayMode);
        }
        else if (property == m_propMeshVsShowEdges) {
            auto meshVisu = static_cast<MeshVS_Mesh*>(gpxObject.operator->());
            meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, value.toBool());
            meshVisu->Redisplay(Standard_True); // All modes
            cxt->UpdateCurrentViewer();
        }
        else if (property == m_propMeshVsShowNodes) {
            auto meshVisu = static_cast<MeshVS_Mesh*>(gpxObject.operator->());
            meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, value.toBool());
            meshVisu->Redisplay(Standard_True); // All modes
            cxt->UpdateCurrentViewer();
        }
        else if (property == m_propMaterial) {
            const int matId = value.toInt();
            const Graphic3d_NameOfMaterial mat = Internal::materials[matId].occMat;
            if (sameType<BRepShapeItem>(itemGpxObject->item)) {
                gpxObject->SetMaterial(mat);
            }
            else if (sameType<StlMeshItem>(itemGpxObject->item)) {
                auto meshVisu = static_cast<MeshVS_Mesh*>(gpxObject.operator->());
                meshVisu->GetDrawer()->SetMaterial(
                            MeshVS_DA_FrontMaterial, Graphic3d_MaterialAspect(mat));
                meshVisu->Redisplay(Standard_True); // All modes
            }
            cxt->UpdateCurrentViewer();
        }
        else if (property == m_propColor) {
            const Quantity_Color color =
                    occ::QtUtils::toOccColor(qvariant_cast<QColor>(value));
            if (sameType<BRepShapeItem>(itemGpxObject->item)) {
                cxt->SetColor(gpxObject, color);
            }
            else if (sameType<StlMeshItem>(itemGpxObject->item)) {
                auto meshVisu = static_cast<MeshVS_Mesh*>(gpxObject.operator->());
                meshVisu->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, color);
                meshVisu->Redisplay(Standard_True); // All modes
                cxt->UpdateCurrentViewer();
            }
        }
    }
}

void DocumentView::connectPropertyValueChangeSignals(bool on)
{
    if (on) {
        QObject::connect(
                    m_varPropMgr, &QtVariantPropertyManager::valueChanged,
                    this, &DocumentView::onQVariantPropertyValueChanged,
                    Qt::UniqueConnection);
    }
    else {
        QObject::disconnect(
                    m_varPropMgr, &QtVariantPropertyManager::valueChanged,
                    this, &DocumentView::onQVariantPropertyValueChanged);
    }
}

const DocumentView::Item_GpxObject *DocumentView::selectedDocumentItem() const
{
    const QList<QTreeWidgetItem*> selectedTreeItems =
            m_ui->treeWidget_Document->selectedItems();
    if (selectedTreeItems.size() == 1) {
        auto itFind = m_mapTreeItemGpxObject.find(selectedTreeItems.first());
        return itFind != m_mapTreeItemGpxObject.cend() ? &(itFind->second) : nullptr;
    }
    return nullptr;
}

DocumentView::Item_GpxObject::Item_GpxObject(
        const DocumentItem *pItem,
        const Handle_AIS_InteractiveObject &pGpxObject)
    : item(pItem), gpxObject(pGpxObject)
{
}


} // namespace Mayo
