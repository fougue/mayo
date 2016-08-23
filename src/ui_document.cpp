#include "ui_document.h"

#include "document.h"
#include "brep_shape_item.h"
#include "stl_mesh_item.h"
#include "qt_occ_view.h"
#include "qt_occ_view_controller.h"
#include "fougtools/qttools/gui/qwidget_utils.h"

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

UiDocument::UiDocument(Document *doc, QWidget *parent)
    : QWidget(parent),
      m_document(doc),
      m_v3dViewer(Internal::createOccViewer()),
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer)),
      m_view(new QtOccView(m_v3dViewer, this))
{
    Q_ASSERT(doc != nullptr);

    new QtOccViewController(m_view);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    auto btnFitAll = Internal::createViewBtn(this, "fit_all", tr("Fit All"));
    auto btnViewIso = Internal::createViewBtn(this, "view_axo", tr("Isometric"));
    auto btnViewBack = Internal::createViewBtn(this, "view_back", tr("Back"));
    auto btnViewFront = Internal::createViewBtn(this, "view_front", tr("Front"));
    auto btnViewLeft = Internal::createViewBtn(this, "view_left", tr("Left"));
    auto btnViewRight = Internal::createViewBtn(this, "view_right", tr("Right"));
    auto btnViewTop = Internal::createViewBtn(this, "view_top", tr("Top"));
    auto btnViewBottom = Internal::createViewBtn(this, "view_bottom", tr("Bottom"));
    btnFitAll->move(0, 0);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewIso, btnFitAll);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBack, btnViewIso);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewFront, btnViewBack);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewLeft, btnViewFront);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewRight, btnViewLeft);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewTop, btnViewRight);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBottom, btnViewTop);

    Internal::connectViewProjBtn(btnViewIso, m_view, V3d_XposYnegZpos);
    Internal::connectViewProjBtn(btnViewBack, m_view, V3d_Xneg);
    Internal::connectViewProjBtn(btnViewFront, m_view, V3d_Xpos);
    Internal::connectViewProjBtn(btnViewLeft, m_view, V3d_Ypos);
    Internal::connectViewProjBtn(btnViewRight, m_view, V3d_Yneg);
    Internal::connectViewProjBtn(btnViewTop, m_view, V3d_Zpos);
    Internal::connectViewProjBtn(btnViewBottom, m_view, V3d_Zneg);
    QObject::connect(
                btnFitAll, &QAbstractButton::clicked, m_view, &QtOccView::fitAll);
    QObject::connect(
                doc, &Document::partImported, this, &UiDocument::onPartImported);
}

const Document *UiDocument::document() const
{
    return m_document;
}

Document *UiDocument::document()
{
    return m_document;
}

void UiDocument::onPartImported(const PartItem* partItem)
{
    if (sameType<BRepShapeItem>(partItem)) {
        auto brepShapeItem = static_cast<const BRepShapeItem*>(partItem);
        Handle_AIS_Shape aisShape = new AIS_Shape(brepShapeItem->brepShape());
        aisShape->SetMaterial(Graphic3d_NOM_PLASTIC);
        aisShape->SetDisplayMode(AIS_Shaded);
        aisShape->SetColor(Quantity_Color(0.33, 0.33, 0.33, Quantity_TOC_RGB));
        aisShape->Attributes()->SetFaceBoundaryDraw(Standard_True);
        m_aisContext->Display(aisShape);
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
        //meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, Standard_True);
        meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, Standard_False);
        meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, Standard_False);
        meshVisu->GetDrawer()->SetMaterial(
                    MeshVS_DA_FrontMaterial,
                    Graphic3d_MaterialAspect(Graphic3d_NOM_PLASTIC));
        meshVisu->SetColor(Quantity_NOC_YELLOW);
        //meshVisu->SetDisplayMode(MeshVS_DMF_WireFrame);
        meshVisu->SetDisplayMode(MeshVS_DMF_Shading);
        // Wireframe as default hilight mode
        meshVisu->SetHilightMode(MeshVS_DMF_WireFrame);
        //meshVisu->SetInfiniteState(Standard_True);
        meshVisu->SetMeshSelMethod(MeshVS_MSM_PRECISE);

        m_aisContext->Display(meshVisu);
    }
}

} // namespace Mayo
