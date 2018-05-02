/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "gui_document.h"

#include "bnd_utils.h"
#include "document.h"
#include "document_item.h"
#include "gpx_document_item.h"
#include "gpx_mesh_item.h"
#include "gpx_utils.h"
#include "gpx_xde_document_item.h"
#include "mesh_item.h"
#include "xde_document_item.h"

#include <AIS_Trihedron.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include <cassert>

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
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer)),
      m_v3dView(m_v3dViewer->CreateView())
{
    assert(doc != nullptr);

    // 3D view - Enable anti-aliasing with MSAA
    m_v3dView->ChangeRenderingParams().IsAntialiasingEnabled = true;
    m_v3dView->ChangeRenderingParams().NbMsaaSamples = 4;
    // 3D view - Set gradient background
    m_v3dView->SetBgGradientColors(
                Quantity_Color(0.5, 0.58, 1., Quantity_TOC_RGB),
                Quantity_NOC_WHITE,
                Aspect_GFM_VER);
    // 3D view - Add shaded trihedron located in the bottom-left corner
    m_v3dView->TriedronDisplay(
                Aspect_TOTP_LEFT_LOWER,
                Quantity_NOC_GRAY50,
                0.075,
                V3d_ZBUFFER);
    // 3D scene - Add trihedron placed at the origin
    m_aisContext->Display(Internal::createOriginTrihedron(), true);

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
    auto itFound = std::find_if(
                m_vecDocItemGpx.cbegin(),
                m_vecDocItemGpx.cend(),
                [=](const DocumentItem_Gpx& pair) { return pair.item == item; });
    return itFound != m_vecDocItemGpx.cend() ? itFound->gpx : nullptr;
}

const Bnd_Box &GuiDocument::gpxBoundingBox() const
{
    return m_gpxBoundingBox;
}

void GuiDocument::onItemAdded(DocumentItem *item)
{
    const DocumentItem_Gpx pair = { item, Internal::createGpxForItem(item) };
    const Handle_AIS_InteractiveObject aisObject = pair.gpx->handleGpxObject();
    m_aisContext->Display(aisObject, true);
    m_vecDocItemGpx.emplace_back(std::move(pair));
    GpxUtils::V3dView_fitAll(m_v3dView);
    BndUtils::add(&m_gpxBoundingBox, BndUtils::get(aisObject));
    emit gpxBoundingBoxChanged(m_gpxBoundingBox);
}

void GuiDocument::onItemErased(const DocumentItem *item)
{
    auto itFound = std::find_if(
                m_vecDocItemGpx.begin(),
                m_vecDocItemGpx.end(),
                [=](const DocumentItem_Gpx& pair) { return pair.item == item; });
    if (itFound != m_vecDocItemGpx.end()) {
        // Delete gpx item
        GpxDocumentItem* gpxDocItem = itFound->gpx;
        GpxUtils::AisContext_eraseObject(m_aisContext, gpxDocItem->handleGpxObject());
        delete gpxDocItem;
        m_vecDocItemGpx.erase(itFound);

        // Recompute bounding box
        m_gpxBoundingBox.SetVoid();
        for (const DocumentItem_Gpx& pair : m_vecDocItemGpx) {
            const Bnd_Box otherBox = BndUtils::get(pair.gpx->handleGpxObject());
            BndUtils::add(&m_gpxBoundingBox, otherBox);
        }
        emit gpxBoundingBoxChanged(m_gpxBoundingBox);
    }
}

} // namespace Mayo
