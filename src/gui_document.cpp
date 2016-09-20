#include "gui_document.h"

#include "document.h"
#include "document_item.h"
#include "gpx_document_item.h"
#include "widget_gui_document_view3d.h"
#include "widget_occ_view.h"

#include "gpx_brep_shape_item.h"
#include "gpx_stl_mesh_item.h"
#include "brep_shape_item.h"
#include "stl_mesh_item.h"

#include <Standard_Version.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#if OCC_VERSION_HEX <= 0x60701
#  include <Graphic3d.hxx>
#else
#  include <OpenGl_GraphicDriver.hxx>
#endif
#include <SelectMgr_SelectionManager.hxx>
#include <TCollection_ExtendedString.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include <cassert>

namespace Mayo {

namespace Internal {

template<typename ITEM> struct ItemTraits { };
template<> struct ItemTraits<BRepShapeItem> { typedef GpxBRepShapeItem GpxType; };
template<> struct ItemTraits<StlMeshItem> { typedef GpxStlMeshItem GpxType; };

template<typename ITEM>
bool createGpxIfItemOfType(GpxDocumentItem** gpx, DocumentItem* item)
{
    if (*gpx == nullptr && sameType<ITEM>(item)) {
        typedef typename ItemTraits<ITEM>::GpxType GpxType;
        *gpx = new GpxType(static_cast<ITEM*>(item));
        return true;
    }
    return false;
}

static GpxDocumentItem* createGpxForItem(DocumentItem* item)
{
    GpxDocumentItem* gpx = nullptr;
    createGpxIfItemOfType<BRepShapeItem>(&gpx, item);
    createGpxIfItemOfType<StlMeshItem>(&gpx, item);
    return gpx;
}

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

void eraseGpxObjectFromContext(
        const Handle_AIS_InteractiveObject &object,
        const Handle_AIS_InteractiveContext &context)
{
    if (!object.IsNull()) {
        context->Erase(object, Standard_False);
        context->Remove(object, Standard_False);
#if OCC_VERSION_HEX < 0x060900
        context->Clear(object, Standard_False); // Note: Remove() can be used too
#else
        context->ClearPrs(object, 0, Standard_False);
#endif
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

    QObject::connect(
                doc, &Document::itemAdded, this, &GuiDocument::onItemAdded);
    QObject::connect(
                doc, &Document::itemErased, this, &GuiDocument::onItemErased);
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

void GuiDocument::onItemAdded(DocumentItem *item)
{
    const DocumentItem_Gpx pair = { item, Internal::createGpxForItem(item) };
    m_vecDocItemGpx.emplace_back(std::move(pair));
    m_aisContext->Display(pair.gpx->handleGpxObject());
    m_guiDocView3d->widgetOccView()->fitAll();
}

void GuiDocument::onItemErased(const DocumentItem *item)
{
    auto itFound = std::find_if(
                m_vecDocItemGpx.begin(),
                m_vecDocItemGpx.end(),
                [=](const DocumentItem_Gpx& pair) { return pair.item == item; });
    if (itFound != m_vecDocItemGpx.end()) {
        Internal::eraseGpxObjectFromContext(
                    itFound->gpx->handleGpxObject(), m_aisContext);
        delete itFound->gpx;
        m_vecDocItemGpx.erase(itFound);
    }
}

} // namespace Mayo
