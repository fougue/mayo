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
#include "widget_gui_document_view3d.h"
#include "widget_occ_view.h"
#include "gpx_xde_document_item.h"
#include "gpx_mesh_item.h"
#include "xde_document_item.h"
#include "mesh_item.h"

#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <SelectMgr_SelectionManager.hxx>
#include <TCollection_ExtendedString.hxx>
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

void eraseGpxObjectFromContext(
        const Handle_AIS_InteractiveObject &object,
        const Handle_AIS_InteractiveContext &context)
{
    if (!object.IsNull()) {
        context->Erase(object, Standard_False);
        context->Remove(object, Standard_False);
        context->ClearPrs(object, 0, Standard_False);
        context->SelectionManager()->Remove(object);

        Handle_AIS_InteractiveObject objectHCopy = object;
        while (!objectHCopy.IsNull())
            objectHCopy.Nullify();
    }
}

} // namespace Internal

GuiDocument::GuiDocument(Document *doc)
    : m_document(doc),
      m_v3dViewer(Internal::createOccViewer()),
      m_aisContext(new AIS_InteractiveContext(m_v3dViewer)),
      m_guiDocView3d(new WidgetGuiDocumentView3d(this))
{
    assert(doc != nullptr);

    m_guiDocView3d->widgetOccView()->setOccV3dViewer(m_v3dViewer);

    QObject::connect(doc, &Document::itemAdded, this, &GuiDocument::onItemAdded);
    QObject::connect(doc, &Document::itemErased, this, &GuiDocument::onItemErased);
}

Document *GuiDocument::document() const
{
    return m_document;
}

WidgetGuiDocumentView3d *GuiDocument::widgetView3d() const
{
    return m_guiDocView3d;
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
    m_guiDocView3d->widgetOccView()->fitAll();

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
        Internal::eraseGpxObjectFromContext(
                    itFound->gpx->handleGpxObject(), m_aisContext);
        delete itFound->gpx;
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
