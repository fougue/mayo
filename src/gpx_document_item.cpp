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

void GpxDocumentItem::setVisible(bool on)
{
    Mayo_PropertyChangedBlocker(this);
    this->propertyIsVisible.setValue(on);
}

void GpxDocumentItem::activateSelection(int /*mode*/)
{
}

std::vector<Handle_SelectMgr_EntityOwner> GpxDocumentItem::entityOwners(int /*mode*/) const
{
    return std::vector<Handle_SelectMgr_EntityOwner>();
}

void GpxDocumentItem::onPropertyChanged(Property *prop)
{
    if (prop == &this->propertyIsVisible) {
        this->setVisible(this->propertyIsVisible.value());
        this->context()->UpdateCurrentViewer();
    }
}

void GpxDocumentItem::getEntityOwners(
                    const Handle_AIS_InteractiveContext& ctx,
                    const Handle_AIS_InteractiveObject& obj,
                    int mode,
                    std::vector<Handle_SelectMgr_EntityOwner>* vec)
{
    opencascade::handle<SelectMgr_IndexedMapOfOwner> mapEntityOwner;
    ctx->EntityOwners(mapEntityOwner, obj, mode);
    vec->reserve(vec->capacity() + mapEntityOwner->Extent());
    for (auto it = mapEntityOwner->cbegin(); it != mapEntityOwner->cend(); ++it)
        vec->push_back(std::move(*it));
}

} // namespace Mayo
