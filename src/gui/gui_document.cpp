/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document.h"

#include "../app/theme.h" // TODO Remove this dependency
#include "../base/application_item.h"
#include "../base/bnd_utils.h"
#include "../base/brep_utils.h"
#include "../base/document.h"
#include "../gpx/gpx_utils.h"
#include "../graphics/graphics_entity_driver_table.h"

#include <fougtools/occtools/qt_utils.h>

#include <AIS_Trihedron.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <StdSelect_BRepOwner.hxx>

namespace Mayo {

namespace Internal {

static Handle_V3d_Viewer createOccViewer()
{
    Handle_Aspect_DisplayConnection dispConnection;
#if (!defined(Q_OS_WIN32) && (!defined(Q_OS_MAC) || defined(MACOSX_USE_GLX)))
    dispConnection = new Aspect_DisplayConnection(std::getenv("DISPLAY"));
#endif
    Handle_Graphic3d_GraphicDriver gpxDriver = new OpenGl_GraphicDriver(dispConnection);
    Handle_V3d_Viewer viewer = new V3d_Viewer(gpxDriver);
    viewer->SetDefaultViewSize(1000.);
    viewer->SetDefaultViewProj(V3d_XposYnegZpos);
    viewer->SetComputedMode(Standard_True);
    viewer->SetDefaultComputedMode(Standard_True);
//    viewer->SetDefaultVisualization(V3d_ZBUFFER);
//    viewer->SetDefaultShadingModel(V3d_GOURAUD);
    viewer->SetDefaultLights();
    viewer->SetLightOn();

    return viewer;
}

static Handle_AIS_Trihedron createOriginTrihedron()
{
    Handle_Geom_Axis2Placement axis = new Geom_Axis2Placement(gp::XOY());
    Handle_AIS_Trihedron aisTrihedron = new AIS_Trihedron(axis);
    aisTrihedron->SetDatumDisplayMode(Prs3d_DM_Shaded);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_XArrow, Quantity_NOC_RED2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_YArrow, Quantity_NOC_GREEN2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_ZArrow, Quantity_NOC_BLUE2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_XAxis, Quantity_NOC_RED2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_YAxis, Quantity_NOC_GREEN2);
    aisTrihedron->SetDatumPartColor(Prs3d_DP_ZAxis, Quantity_NOC_BLUE2);
    aisTrihedron->SetLabel(Prs3d_DP_XAxis, "");
    aisTrihedron->SetLabel(Prs3d_DP_YAxis, "");
    aisTrihedron->SetLabel(Prs3d_DP_ZAxis, "");
    //aisTrihedron->SetTextColor(Quantity_NOC_GRAY40);
    aisTrihedron->SetSize(60);
    Handle_Graphic3d_TransformPers trsf =
            new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, axis->Ax2().Location());
    aisTrihedron->SetTransformPersistence(trsf);
    aisTrihedron->SetInfiniteState(true);
    return aisTrihedron;
}

} // namespace Internal

GuiDocument::GuiDocument(const DocumentPtr& doc)
    : m_document(doc),
      m_v3dViewer(Internal::createOccViewer()),
      m_v3dView(m_v3dViewer->CreateView()),
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer)),
      m_aisOriginTrihedron(Internal::createOriginTrihedron())
{
    Expects(!doc.IsNull());

    //m_v3dView->SetShadingModel(V3d_PHONG);
    // 3D view - Enable anti-aliasing with MSAA
    m_v3dView->ChangeRenderingParams().IsAntialiasingEnabled = true;
    m_v3dView->ChangeRenderingParams().NbMsaaSamples = 4;
    // 3D view - Set gradient background
    m_v3dView->SetBgGradientColors(
                occ::QtUtils::toOccColor(
                    mayoTheme()->color(Theme::Color::View3d_BackgroundGradientStart)),
                occ::QtUtils::toOccColor(
                    mayoTheme()->color(Theme::Color::View3d_BackgroundGradientEnd)),
                Aspect_GFM_VER);
    // 3D view - Add shaded trihedron located in the bottom-left corner
    m_v3dView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GRAY50, 0.075, V3d_ZBUFFER);

    for (int i = 0; i < doc->entityCount(); ++i)
        this->mapGraphics(doc->entityTreeNodeId(i));

    QObject::connect(doc.get(), &Document::entityAdded, this, &GuiDocument::onDocumentEntityAdded);
    QObject::connect(
                doc.get(), &Document::entityAboutToBeDestroyed,
                this, &GuiDocument::onDocumentEntityAboutToBeDestroyed);
}

const DocumentPtr& GuiDocument::document() const
{
    return m_document;
}

const Handle_V3d_View& GuiDocument::v3dView() const
{
    return m_v3dView;
}

const Handle_AIS_InteractiveContext& GuiDocument::aisInteractiveContext() const
{
    return m_aisContext;
}

//GpxDocumentItem* GuiDocument::findItemGpx(const DocumentItem* item) const
//{
//    const GuiDocumentItem* guiDocItem = this->findGuiDocumentItem(item);
//    return guiDocItem ? guiDocItem->gpxDocItem.get() : nullptr;
//}

const Bnd_Box& GuiDocument::gpxBoundingBox() const
{
    return m_gpxBoundingBox;
}

void GuiDocument::toggleItemSelected(const ApplicationItem& appItem)
{
    if (appItem.document() != this->document())
        return;

    if (appItem.isDocumentTreeNode()) {
        const DocumentPtr doc = appItem.document();
        const DocumentTreeNode& docTreeNode = appItem.documentTreeNode();
        if (!doc.IsNull() && XCaf::isShape(docTreeNode.label())) {
            const TopLoc_Location shapeLoc = doc->xcaf().shapeAbsoluteLocation(docTreeNode.id());
            const TopoDS_Shape shape = XCaf::shape(docTreeNode.label()).Located(shapeLoc);
            std::vector<TopoDS_Face> vecFace;
            if (BRepUtils::moreComplex(shape.ShapeType(), TopAbs_FACE)) {
                BRepUtils::forEachSubFace(shape, [&](const TopoDS_Face& face) {
                    vecFace.push_back(face);
                });
            }
            else if (shape.ShapeType() == TopAbs_FACE) {
                vecFace.push_back(TopoDS::Face(shape));
            }

//            for (const TopoDS_Face& face : vecFace) {
//                auto brepOwner = guiItem->findBrepOwner(face);
//                if (!brepOwner.IsNull())
//                    m_aisContext->AddOrRemoveSelected(brepOwner, false);
//            }
        }
    }
}

void GuiDocument::clearItemSelection()
{
    m_aisContext->ClearSelected(false);
}

bool GuiDocument::isOriginTrihedronVisible() const
{
    return m_aisContext->IsDisplayed(m_aisOriginTrihedron);
}

void GuiDocument::toggleOriginTrihedronVisibility()
{
    if (this->isOriginTrihedronVisible())
        m_aisContext->Erase(m_aisOriginTrihedron, false);
    else
        m_aisContext->Display(m_aisOriginTrihedron, false);
}

void GuiDocument::updateV3dViewer()
{
    m_aisContext->UpdateCurrentViewer();
}

void GuiDocument::onDocumentEntityAdded(TreeNodeId entityTreeNodeId)
{
    this->mapGraphics(entityTreeNodeId);
    emit gpxBoundingBoxChanged(m_gpxBoundingBox);
}

void GuiDocument::onDocumentEntityAboutToBeDestroyed(TreeNodeId entityTreeNodeId)
{
    auto itFound = std::find_if(
                m_vecGraphicsItem.begin(),
                m_vecGraphicsItem.end(),
                [=](const GraphicsItem& item) { return item.entityTreeNodeId == entityTreeNodeId; });
    if (itFound != m_vecGraphicsItem.end()) {
        const GraphicsEntity& gfxEntity = itFound->graphicsEntity;
        GpxUtils::AisContext_eraseObject(m_aisContext, gfxEntity.aisObject());
        m_vecGraphicsItem.erase(itFound);
        this->updateV3dViewer();

        // Recompute bounding box
        m_gpxBoundingBox.SetVoid();
        for (const GraphicsItem& item : m_vecGraphicsItem) {
            const Bnd_Box entityBndBox = GpxUtils::AisObject_boundingBox(item.graphicsEntity.aisObject());
            BndUtils::add(&m_gpxBoundingBox, entityBndBox);
        }

        emit gpxBoundingBoxChanged(m_gpxBoundingBox);
    }
}

std::vector<Handle_SelectMgr_EntityOwner> GuiDocument::selectedEntityOwners() const
{
    std::vector<Handle_SelectMgr_EntityOwner> vecOwner;
    m_aisContext->InitSelected();
    while (m_aisContext->MoreSelected()) {
        vecOwner.push_back(m_aisContext->SelectedOwner());
        m_aisContext->NextSelected();
    }

    return vecOwner;
}

void GuiDocument::mapGraphics(TreeNodeId entityTreeNodeId)
{
    GraphicsItem item;
    const TDF_Label entityLabel = m_document->modelTree().nodeData(entityTreeNodeId);
    item.graphicsEntity = GraphicsEntityDriverTable::instance()->createEntity(entityLabel);
    if (item.graphicsEntity.aisObject().IsNull())
        return;

    item.graphicsEntity.setAisContext(m_aisContext);
    item.graphicsEntity.setVisible(true);
    item.entityTreeNodeId = entityTreeNodeId;
    m_aisContext->UpdateCurrentViewer();
//    if (sameType<XdeDocumentItem>(item)) {
//        gpxItem->activateSelection(GpxXdeDocumentItem::SelectVertex);
//        gpxItem->activateSelection(GpxXdeDocumentItem::SelectEdge);
//        gpxItem->activateSelection(GpxXdeDocumentItem::SelectWire);
//        gpxItem->activateSelection(GpxXdeDocumentItem::SelectFace);
//        gpxItem->activateSelection(GpxXdeDocumentItem::SelectShell);
//        gpxItem->activateSelection(GpxXdeDocumentItem::SelectSolid);
//        guiItem.vecGpxEntityOwner = gpxItem->entityOwners(GpxXdeDocumentItem::SelectFace);
//    }

    GpxUtils::V3dView_fitAll(m_v3dView);
    const Bnd_Box itemBndBox = GpxUtils::AisObject_boundingBox(item.graphicsEntity.aisObject());
    BndUtils::add(&m_gpxBoundingBox, itemBndBox);
    m_vecGraphicsItem.emplace_back(std::move(item));
}

//Handle_SelectMgr_EntityOwner
//GuiDocument::GuiDocumentItem::findBrepOwner(const TopoDS_Face& face) const
//{
//    for (const Handle_SelectMgr_EntityOwner& owner : vecGpxEntityOwner) {
//        auto brepOwner = Handle_StdSelect_BRepOwner::DownCast(owner);
//        if (!brepOwner.IsNull() && brepOwner->Shape() == face)
//            return owner;
//    }

//    return Handle_SelectMgr_EntityOwner();
//}

} // namespace Mayo
