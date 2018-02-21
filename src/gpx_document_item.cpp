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

#include "gpx_document_item.h"

#include "options.h"
#include "fougtools/occtools/qt_utils.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <QtCore/QCoreApplication>

namespace Mayo {

GpxDocumentItem::GpxDocumentItem()
    : propertyIsVisible(this, tr("Visible")),
      propertyMaterial(this, tr("Material"), &enum_Graphic3dNameOfMaterial()),
      propertyColor(this, tr("Color"))
{
    Mayo_PropertyChangedBlocker(this);
    this->propertyIsVisible.setValue(true);
}

void GpxDocumentItem::onPropertyChanged(Property *prop)
{
    if (prop == &this->propertyIsVisible) {
        Handle_AIS_InteractiveContext cxt = this->handleGpxObject()->GetContext();
        if (this->propertyIsVisible.value())
            cxt->Display(this->handleGpxObject(), Standard_True);
        else
            cxt->Erase(this->handleGpxObject(), Standard_True);
    }
}

void GpxDocumentItem::initForGpxBRepShape(
        const Handle_AIS_InteractiveObject& hndGpx)
{
    const Options* opts = Options::instance();
    hndGpx->SetMaterial(opts->brepShapeDefaultMaterial());
    hndGpx->SetDisplayMode(AIS_Shaded);
    hndGpx->SetColor(occ::QtUtils::toOccColor(opts->brepShapeDefaultColor()));
    hndGpx->Attributes()->SetFaceBoundaryDraw(Standard_True);
    hndGpx->Attributes()->SetIsoOnTriangulation(Standard_True);

    Mayo_PropertyChangedBlocker(this);
    this->propertyMaterial.setValue(opts->brepShapeDefaultMaterial());

    Quantity_Color color;
    hndGpx->Color(color);
    this->propertyColor.setValue(color);
}

void GpxBRepShapeCommonProperties::initCommonProperties(
        PropertyOwner *owner, const Handle_AIS_InteractiveObject& hndGpx)
{
    Mayo_PropertyChangedBlocker(owner);
    this->propertyTransparency.setValue(
                static_cast<int>(hndGpx->Transparency() * 100));
    this->propertyDisplayMode.setValue(hndGpx->DisplayMode());
    this->propertyShowFaceBoundary.setValue(
                hndGpx->Attributes()->FaceBoundaryDraw() == Standard_True);
}

GpxBRepShapeCommonProperties::GpxBRepShapeCommonProperties(PropertyOwner* owner)
    : propertyTransparency(owner, tr("Transparency"), 0, 100, 5),
      propertyDisplayMode(owner, tr("Display mode"), &enum_DisplayMode()),
      propertyShowFaceBoundary(owner, tr("Show face boundary"))
{
}

void GpxBRepShapeCommonProperties::handleCommonPropertyChange(
        Property *prop, const Handle_AIS_InteractiveObject &hndGpx)
{
    Handle_AIS_InteractiveContext cxt = hndGpx->GetContext();
    if (prop == &this->propertyTransparency) {
        cxt->SetTransparency(
                    hndGpx, this->propertyTransparency.value() / 100., Standard_True);
    }
    else if (prop == &this->propertyDisplayMode) {
        cxt->SetDisplayMode(
                    hndGpx, this->propertyDisplayMode.value(), Standard_True);
    }
    else if (prop == &this->propertyShowFaceBoundary) {
        hndGpx->Attributes()->SetFaceBoundaryDraw(
                    this->propertyShowFaceBoundary.value());
        hndGpx->Redisplay(Standard_True); // All modes
        cxt->UpdateCurrentViewer();
    }
}

void GpxBRepShapeCommonProperties::handlePropertyMaterial(
        PropertyEnumeration *prop, const Handle_AIS_InteractiveObject &hndGpx)
{
    Handle_AIS_InteractiveContext cxt = hndGpx->GetContext();
    hndGpx->SetMaterial(prop->valueAs<Graphic3d_NameOfMaterial>());
    cxt->UpdateCurrentViewer();
}

void GpxBRepShapeCommonProperties::handlePropertyColor(
        PropertyOccColor *prop, const Handle_AIS_InteractiveObject &hndGpx)
{
    Handle_AIS_InteractiveContext cxt = hndGpx->GetContext();
    hndGpx->SetColor(prop->value());
    if (this->propertyShowFaceBoundary.value()) {
        hndGpx->Redisplay(Standard_True); // All modes
        cxt->UpdateCurrentViewer();
    }
}

const Enumeration &GpxBRepShapeCommonProperties::enum_DisplayMode()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.map(AIS_Shaded, tr("Shaded"));
        enumeration.map(AIS_WireFrame, tr("Wireframe"));
    }
    return enumeration;
}

} // namespace Mayo
