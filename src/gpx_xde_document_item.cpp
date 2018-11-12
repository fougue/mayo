/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gpx_xde_document_item.h"

#include "bnd_utils.h"
#include "gpx_utils.h"
#include "options.h"

#include <fougtools/occtools/qt_utils.h>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <QtCore/QCoreApplication>
#include <cassert>

#include "span.h"

namespace Mayo {

GpxXdeDocumentItem::GpxXdeDocumentItem(XdeDocumentItem* item)
    : m_xdeDocItem(item),
      propertyTransparency(this, tr("Transparency"), 0, 100, 5),
      propertyDisplayMode(this, tr("Display mode"), &enum_DisplayMode()),
      propertyShowFaceBoundary(this, tr("Show face boundary"))
{
    // XCAFPrs_AISObject requires a root label containing a TopoDS_Shape
    // If the XDE document as many free top-level shapes then there is a problem
    // Doesn't work :
    //     XCAFDoc_DocumentTool::ShapesLabel()
    //     XCAFDoc_ShapeTool::BaseLabel()
    // The only viable solution(instead of creating a root shape label containing
    // the free top-level shapes) is to have an AIS object per free top-level
    // shape.
    const std::vector<TDF_Label> vecFreeShape = item->topLevelFreeShapes();
    if (!vecFreeShape.empty()) {
        m_vecXdeGpx.reserve(vecFreeShape.size());
        const Options* opts = Options::instance();
        Mayo_PropertyChangedBlocker(this);
        for (const TDF_Label& label : vecFreeShape) {
            Handle_XCAFPrs_AISObject gpx = new XCAFPrs_AISObject(label);
            gpx->SetMaterial(opts->brepShapeDefaultMaterial());
            gpx->SetDisplayMode(AIS_Shaded);
            gpx->SetColor(occ::QtUtils::toOccColor(opts->brepShapeDefaultColor()));
            gpx->Attributes()->SetFaceBoundaryDraw(true);
            gpx->Attributes()->SetIsoOnTriangulation(true);
            this->propertyMaterial.setValue(opts->brepShapeDefaultMaterial());
            Quantity_Color color;
            gpx->Color(color);
            this->propertyColor.setValue(color);
            this->propertyDisplayMode.setValue(gpx->DisplayMode());
            this->propertyTransparency.setValue(
                        static_cast<int>(gpx->Transparency() * 100));
            this->propertyShowFaceBoundary.setValue(
                        gpx->Attributes()->FaceBoundaryDraw());
            m_vecXdeGpx.push_back(gpx);
        }
    }
    else { // Dummy
        Handle_XCAFPrs_AISObject gpx = new XCAFPrs_AISObject(item->cafDoc()->Main());
        m_vecXdeGpx.push_back(gpx);
    }
}

GpxXdeDocumentItem::~GpxXdeDocumentItem()
{
    for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
        GpxUtils::AisContext_eraseObject(this->context(), obj);
}

XdeDocumentItem *GpxXdeDocumentItem::documentItem() const
{
    return m_xdeDocItem;
}

void GpxXdeDocumentItem::display()
{
    for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
        this->context()->Display(obj, false);
}

void GpxXdeDocumentItem::hide()
{
    for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
        this->context()->Erase(obj, false);
}

void GpxXdeDocumentItem::activateSelection(int mode)
{
    for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
        this->context()->Activate(obj, mode);
}

std::vector<Handle_SelectMgr_EntityOwner> GpxXdeDocumentItem::entityOwners(int mode) const
{
    std::vector<Handle_SelectMgr_EntityOwner> vecOwner;
    for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
        GpxDocumentItem::getEntityOwners(this->context(), obj, mode, &vecOwner);
    return vecOwner;
}

Bnd_Box GpxXdeDocumentItem::boundingBox() const
{
    Bnd_Box bndBox;
    for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
        bndBox.Add(BndUtils::get(obj));
    return bndBox;
}

void GpxXdeDocumentItem::onPropertyChanged(Property* prop)
{
    if (prop == &this->propertyMaterial) {
        for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
            obj->SetMaterial(this->propertyMaterial.valueAs<Graphic3d_NameOfMaterial>());
        this->context()->UpdateCurrentViewer();
    }
    else if (prop == &this->propertyColor) {
        for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx) {
            obj->SetColor(this->propertyColor.value());
            if (this->propertyShowFaceBoundary.value())
                obj->Redisplay(true); // All modes
        }
        this->context()->UpdateCurrentViewer();
    }
    if (prop == &this->propertyTransparency) {
        const double factor = this->propertyTransparency.value() / 100.;
        for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
            this->context()->SetTransparency(obj, factor, false);
        this->context()->UpdateCurrentViewer();
    }
    else if (prop == &this->propertyDisplayMode) {
        for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx)
            this->context()->SetDisplayMode(obj, this->propertyDisplayMode.value(), false);
        this->context()->UpdateCurrentViewer();
    }
    else if (prop == &this->propertyShowFaceBoundary) {
        for (const Handle_XCAFPrs_AISObject& obj : m_vecXdeGpx) {
            obj->Attributes()->SetFaceBoundaryDraw(this->propertyShowFaceBoundary.value());
            obj->Redisplay(true); // All modes
        }
        this->context()->UpdateCurrentViewer();
    }
    GpxDocumentItem::onPropertyChanged(prop);
}

const Enumeration& GpxXdeDocumentItem::enum_DisplayMode()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.map(AIS_Shaded, tr("Shaded"));
        enumeration.map(AIS_WireFrame, tr("Wireframe"));
    }
    return enumeration;
}

} // namespace Mayo
