/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gpx_xde_document_item.h"

#include "options.h"

#include <fougtools/occtools/qt_utils.h>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <QtCore/QCoreApplication>
#include <cassert>

namespace Mayo {

GpxXdeDocumentItem::GpxXdeDocumentItem(XdeDocumentItem* item)
    : m_xdeDocItem(item),
      propertyTransparency(this, tr("Transparency"), 0, 100, 5),
      propertyDisplayMode(this, tr("Display mode"), &enum_DisplayMode()),
      propertyShowFaceBoundary(this, tr("Show face boundary"))
{
    // TODO XCAFPrs_AISObject requires a root label containing a TopoDS_Shape
    // If the XDE document as many free top-level shapes then there is a problem
    // Doesn't work :
    //     XCAFDoc_DocumentTool::ShapesLabel()
    //     XCAFDoc_ShapeTool::BaseLabel()
    // The only viable solution(instead of creating a root shape label containing
    // the free top-level shapes) is to have an AIS object per free top-level
    // shape.
    const std::vector<TDF_Label> vecFreeShape = item->topLevelFreeShapes();
    assert(vecFreeShape.size() <= 1);
    if (!vecFreeShape.empty()) {
        m_xdeGpx = new XCAFPrs_AISObject(vecFreeShape.front());

        const Options* opts = Options::instance();
        m_xdeGpx->SetMaterial(opts->brepShapeDefaultMaterial());
        m_xdeGpx->SetDisplayMode(AIS_Shaded);
        m_xdeGpx->SetColor(occ::QtUtils::toOccColor(opts->brepShapeDefaultColor()));
        m_xdeGpx->Attributes()->SetFaceBoundaryDraw(true);
        m_xdeGpx->Attributes()->SetIsoOnTriangulation(true);

        Mayo_PropertyChangedBlocker(this);
        this->propertyMaterial.setValue(opts->brepShapeDefaultMaterial());

        Quantity_Color color;
        m_xdeGpx->Color(color);
        this->propertyColor.setValue(color);

        this->propertyDisplayMode.setValue(m_xdeGpx->DisplayMode());
        this->propertyTransparency.setValue(
                    static_cast<int>(m_xdeGpx->Transparency() * 100));
        this->propertyShowFaceBoundary.setValue(
                    m_xdeGpx->Attributes()->FaceBoundaryDraw());
    }
    else { // Dummy
        m_xdeGpx = new XCAFPrs_AISObject(item->cafDoc()->Main());
    }
}

XdeDocumentItem *GpxXdeDocumentItem::documentItem() const
{
    return m_xdeDocItem;
}

Handle_AIS_InteractiveObject GpxXdeDocumentItem::handleGpxObject() const
{
    return m_xdeGpx;
}

void GpxXdeDocumentItem::onPropertyChanged(Property* prop)
{
    Handle_AIS_InteractiveContext cxt = m_xdeGpx->GetContext();
    if (prop == &this->propertyMaterial) {
        m_xdeGpx->SetMaterial(this->propertyMaterial.valueAs<Graphic3d_NameOfMaterial>());
        cxt->UpdateCurrentViewer();
    }
    else if (prop == &this->propertyColor) {
        m_xdeGpx->SetColor(this->propertyColor.value());
        if (this->propertyShowFaceBoundary.value()) {
            m_xdeGpx->Redisplay(true); // All modes
            cxt->UpdateCurrentViewer();
        }
    }
    if (prop == &this->propertyTransparency) {
        cxt->SetTransparency(m_xdeGpx, this->propertyTransparency.value() / 100., true);
    }
    else if (prop == &this->propertyDisplayMode) {
        cxt->SetDisplayMode(m_xdeGpx, this->propertyDisplayMode.value(), true);
    }
    else if (prop == &this->propertyShowFaceBoundary) {
        m_xdeGpx->Attributes()->SetFaceBoundaryDraw(this->propertyShowFaceBoundary.value());
        m_xdeGpx->Redisplay(true); // All modes
        cxt->UpdateCurrentViewer();
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
