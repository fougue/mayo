/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gui_document.h"

#include "application_item.h"
#include "bnd_utils.h"
#include "brep_utils.h"
#include "document.h"
#include "document_item.h"
#include "gpx_document_item.h"
#include "gpx_mesh_item.h"
#include "gpx_utils.h"
#include "gpx_xde_document_item.h"
#include "mesh_item.h"
#include "theme.h"
#include "xde_document_item.h"

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

template<typename ITEM, typename GPX_ITEM>
bool createGpxIfItemOfType(GpxDocumentItem** gpx, DocumentItem* item)
{
    if (*gpx == nullptr && sameType<ITEM>(item)) {
        *gpx = new GPX_ITEM(static_cast<ITEM*>(item));
        return true;
    }
    return false;
}

static GpxDocumentItem* createGpxForItem(DocumentItem* item)
{
    GpxDocumentItem* gpx = nullptr;
    createGpxIfItemOfType<XdeDocumentItem, GpxXdeDocumentItem>(&gpx, item);
    createGpxIfItemOfType<MeshItem, GpxMeshItem>(&gpx, item);
    return gpx;
}

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

GuiDocument::GuiDocument(Document *doc)
    : m_document(doc),
      m_v3dViewer(Internal::createOccViewer()),
      m_v3dView(m_v3dViewer->CreateView()),
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer)),
      m_aisOriginTrihedron(Internal::createOriginTrihedron())
{
    Q_ASSERT(doc != nullptr);

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
    m_v3dView->TriedronDisplay(
                Aspect_TOTP_LEFT_LOWER,
                Quantity_NOC_GRAY50,
                0.075,
                V3d_ZBUFFER);

    for (DocumentItem* docItem : doc->rootItems())
        this->mapGpxItem(docItem);

    QObject::connect(doc, &Document::itemAdded, this, &GuiDocument::onItemAdded);
    QObject::connect(doc, &Document::itemErased, this, &GuiDocument::onItemErased);
}

Document *GuiDocument::document() const
{
    return m_document;
}

const Handle_V3d_View &GuiDocument::v3dView() const
{
    return m_v3dView;
}

const Handle_AIS_InteractiveContext &GuiDocument::aisInteractiveContext() const
{
    return m_aisContext;
}

GpxDocumentItem *GuiDocument::findItemGpx(const DocumentItem *item) const
{
    const GuiDocumentItem* guiDocItem = this->findGuiDocumentItem(item);
    return guiDocItem ? guiDocItem->gpxDocItem.get() : nullptr;
}

const Bnd_Box &GuiDocument::gpxBoundingBox() const
{
    return m_gpxBoundingBox;
}

void GuiDocument::toggleItemSelected(const ApplicationItem &appItem)
{
    if (appItem.document() != this->document())
        return;
    if (appItem.isXdeAssemblyNode()) {
        const XdeAssemblyNode& xdeAsmNode = appItem.xdeAssemblyNode();
        const XdeDocumentItem* xdeItem = xdeAsmNode.ownerDocItem;
        const GuiDocumentItem* guiItem = this->findGuiDocumentItem(xdeItem);
        if (guiItem != nullptr) {
            const TopLoc_Location shapeLoc =
                    xdeItem->shapeAbsoluteLocation(xdeAsmNode.nodeId);
            const TopoDS_Shape shape =
                    XdeDocumentItem::shape(xdeAsmNode.label()).Located(shapeLoc);
            std::vector<TopoDS_Face> vecFace;
            if (BRepUtils::moreComplex(shape.ShapeType(), TopAbs_FACE)) {
                BRepUtils::forEachSubFace(shape, [&](const TopoDS_Face& face) {
                    vecFace.push_back(face);
                });
            }
            else if (shape.ShapeType() == TopAbs_FACE) {
                vecFace.push_back(TopoDS::Face(shape));
            }
            for (const TopoDS_Face& face : vecFace) {
                auto brepOwner = guiItem->findBrepOwner(face);
                if (!brepOwner.IsNull())
                    m_aisContext->AddOrRemoveSelected(brepOwner, false);
            }
        }
    }
#if 0
    else if (appItem.isDocumentItem()) {
        const GpxDocumentItem* gpxItem = this->findItemGpx(appItem.documentItem());
        if (gpxItem != nullptr)
            m_aisContext->AddOrRemoveSelected(gpxItem->handleGpxObject(), false);
    }
    else if (appItem.isDocument()) {
        for (const DocumentItem* docItem : appItem.document()->rootItems()) {
            const GpxDocumentItem* gpxItem = this->findItemGpx(docItem);
            if (gpxItem != nullptr)
                m_aisContext->AddOrRemoveSelected(gpxItem->handleGpxObject(), false);
        }
    }
#endif
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

void GuiDocument::onItemAdded(DocumentItem* item)
{
    this->mapGpxItem(item);
    emit gpxBoundingBoxChanged(m_gpxBoundingBox);
}

void GuiDocument::onItemErased(const DocumentItem *item)
{
    auto itFound = std::find_if(
                m_vecGuiDocumentItem.begin(),
                m_vecGuiDocumentItem.end(),
                [=](const GuiDocumentItem& guiItem) { return guiItem.docItem == item; });
    if (itFound != m_vecGuiDocumentItem.end()) {
        // Delete gpx item
        m_vecGuiDocumentItem.erase(itFound);
        this->updateV3dViewer();

        // Recompute bounding box
        m_gpxBoundingBox.SetVoid();
        for (const GuiDocumentItem& guiItem : m_vecGuiDocumentItem) {
            const Bnd_Box otherBox = guiItem.gpxDocItem->boundingBox();
            BndUtils::add(&m_gpxBoundingBox, otherBox);
        }
        emit gpxBoundingBoxChanged(m_gpxBoundingBox);
    }
}

void GuiDocument::mapGpxItem(DocumentItem *item)
{
    GuiDocumentItem guiItem(item, Internal::createGpxForItem(item));
    guiItem.gpxDocItem->setContext(m_aisContext);
    guiItem.gpxDocItem->setVisible(true);
    m_aisContext->UpdateCurrentViewer();
    if (sameType<XdeDocumentItem>(item)) {
        guiItem.gpxDocItem->activateSelection(GpxXdeDocumentItem::SelectVertex);
        guiItem.gpxDocItem->activateSelection(GpxXdeDocumentItem::SelectEdge);
        guiItem.gpxDocItem->activateSelection(GpxXdeDocumentItem::SelectWire);
        guiItem.gpxDocItem->activateSelection(GpxXdeDocumentItem::SelectFace);
        guiItem.gpxDocItem->activateSelection(GpxXdeDocumentItem::SelectShell);
        guiItem.gpxDocItem->activateSelection(GpxXdeDocumentItem::SelectSolid);
        guiItem.vecGpxEntityOwner = guiItem.gpxDocItem->entityOwners(GpxXdeDocumentItem::SelectFace);
    }
    GpxUtils::V3dView_fitAll(m_v3dView);
    BndUtils::add(&m_gpxBoundingBox, guiItem.gpxDocItem->boundingBox());
    m_vecGuiDocumentItem.emplace_back(std::move(guiItem));
}

const GuiDocument::GuiDocumentItem*
GuiDocument::findGuiDocumentItem(const DocumentItem *item) const
{
    for (const GuiDocumentItem& guiItem : m_vecGuiDocumentItem) {
        if (guiItem.docItem == item)
            return &guiItem;
    }
    return nullptr;
}

GuiDocument::GuiDocumentItem::GuiDocumentItem(DocumentItem *item, GpxDocumentItem *gpx)
    : docItem(item), gpxDocItem(gpx)
{
}

Handle_SelectMgr_EntityOwner
GuiDocument::GuiDocumentItem::findBrepOwner(const TopoDS_Face& face) const
{
    for (const Handle_SelectMgr_EntityOwner& owner : vecGpxEntityOwner) {
        auto brepOwner = Handle_StdSelect_BRepOwner::DownCast(owner);
        if (!brepOwner.IsNull() && brepOwner->Shape() == face)
            return owner;
    }
    return Handle_SelectMgr_EntityOwner();
}

} // namespace Mayo
