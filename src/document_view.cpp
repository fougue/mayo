#include "document_view.h"

#include "document.h"
#include "brep_shape_item.h"
#include "stl_mesh_item.h"
#include "options.h"
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
    for (const Options::Material& mat : Options::materials())
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

const QtOccView *DocumentView::qtOccView() const
{
    return m_ui->widget_View3d;
}

void DocumentView::onPartImported(const PartItem* partItem)
{
    if (partItem->isNull())
        return;

    const Options* opts = Options::instance();
    Handle_AIS_InteractiveObject aisObject;
    if (sameType<BRepShapeItem>(partItem)) {
        auto brepShapeItem = static_cast<const BRepShapeItem*>(partItem);
        Handle_AIS_Shape aisShape = new AIS_Shape(brepShapeItem->brepShape());
        aisShape->SetMaterial(opts->brepShapeDefaultMaterial());
        aisShape->SetDisplayMode(AIS_Shaded);
        aisShape->SetColor(occ::QtUtils::toOccColor(opts->brepShapeDefaultColor()));
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
        meshVisu->GetDrawer()->SetBoolean(
                    MeshVS_DA_ShowEdges, opts->meshDefaultShowEdges());
        meshVisu->GetDrawer()->SetBoolean(
                    MeshVS_DA_DisplayNodes, opts->meshDefaultShowNodes());
        meshVisu->GetDrawer()->SetMaterial(
                    MeshVS_DA_FrontMaterial,
                    Graphic3d_MaterialAspect(opts->meshDefaultMaterial()));
        meshVisu->GetDrawer()->SetColor(
                    MeshVS_DA_InteriorColor,
                    occ::QtUtils::toOccColor(opts->meshDefaultColor()));
        meshVisu->SetDisplayMode(MeshVS_DMF_Shading);
        // Wireframe as default hilight mode
        meshVisu->SetHilightMode(MeshVS_DMF_WireFrame);
        meshVisu->SetMeshSelMethod(MeshVS_MSM_PRECISE);
        aisObject = meshVisu;
    }

    if (!aisObject.IsNull()) {
        auto treeItem = new QTreeWidgetItem;
        const QString partLabel =
                !partItem->label().isEmpty() ? partItem->label() : tr("<unnamed>");
        treeItem->setText(0, partLabel);

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
                    std::cbegin(Options::materials()),
                    std::cend(Options::materials()),
                    [=](const Options::Material& mat) {
            return mat.code == partMaterial;
        } );
        if (itMat != std::cend(Options::materials()))
            m_propMaterial->setValue(itMat - std::cbegin(Options::materials()));

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
            const Graphic3d_NameOfMaterial mat = Options::materials().at(matId).code;
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
