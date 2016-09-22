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

#include "gpx_brep_shape_item.h"

#include "options.h"
#include "fougtools/occtools/qt_utils.h"

#include <AIS_InteractiveContext.hxx>

namespace Mayo {

GpxBRepShapeItem::GpxBRepShapeItem(BRepShapeItem *item)
    : GpxCovariantDocumentItem(item),
      propertyTransparency(this, tr("Transparency"), 0, 100, 5),
      propertyDisplayMode(this, tr("Display mode"), &enum_DisplayMode()),
      propertyShowFaceBoundary(this, tr("Show face boundary"))
{
    const Options* opts = Options::instance();

    // Create the AIS_Shape object
    Handle_AIS_Shape aisShape = new AIS_Shape(item->brepShape());
    aisShape->SetMaterial(opts->brepShapeDefaultMaterial());
    aisShape->SetDisplayMode(AIS_Shaded);
    aisShape->SetColor(occ::QtUtils::toOccColor(opts->brepShapeDefaultColor()));
    aisShape->Attributes()->SetFaceBoundaryDraw(Standard_True);
    aisShape->Attributes()->SetIsoOnTriangulation(Standard_True);

    m_hndGpxObject = aisShape;

    // Init properties
    Mayo_PropertyChangedBlocker(this);
    this->propertyMaterial.setValue(opts->brepShapeDefaultMaterial());
    this->propertyColor.setValue(m_hndGpxObject->Color());
    this->propertyTransparency.setValue(
                static_cast<int>(m_hndGpxObject->Transparency() * 100));
    this->propertyDisplayMode.setValue(m_hndGpxObject->DisplayMode());
    this->propertyShowFaceBoundary.setValue(
                m_hndGpxObject->Attributes()->FaceBoundaryDraw() == Standard_True);
}

void GpxBRepShapeItem::onPropertyChanged(Property *prop)
{
    Handle_AIS_InteractiveContext cxt = this->gpxObject()->GetContext();
    const Handle_AIS_InteractiveObject& hndGpx = this->handleGpxObject();
    AIS_Shape* ptrGpx = this->gpxObject();
    if (prop == &this->propertyMaterial) {
        ptrGpx->SetMaterial(
                    this->propertyMaterial.valueAs<Graphic3d_NameOfMaterial>());
        cxt->UpdateCurrentViewer();
    }
    else if (prop == &this->propertyColor) {
        ptrGpx->SetColor(this->propertyColor.value());
        if (this->propertyShowFaceBoundary.value()) {
            ptrGpx->Redisplay(Standard_True); // All modes
            cxt->UpdateCurrentViewer();
        }
    }
    else if (prop == &this->propertyTransparency) {
        cxt->SetTransparency(hndGpx, this->propertyTransparency.value() / 100.);
    }
    else if (prop == &this->propertyDisplayMode) {
        cxt->SetDisplayMode(hndGpx, this->propertyDisplayMode.value());
    }
    else if (prop == &this->propertyShowFaceBoundary) {
        ptrGpx->Attributes()->SetFaceBoundaryDraw(
                    this->propertyShowFaceBoundary.value());
        ptrGpx->Redisplay(Standard_True); // All modes
        cxt->UpdateCurrentViewer();
    }
    GpxDocumentItem::onPropertyChanged(prop);
}

const Enumeration &GpxBRepShapeItem::enum_DisplayMode()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.map(AIS_Shaded, tr("Shaded"));
        enumeration.map(AIS_WireFrame, tr("Wireframe"));
    }
    return enumeration;
}

} // namespace Mayo
