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

#pragma once

#include "document_item.h"
#include <TDocStd_Document.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>

namespace Mayo {

class XdeDocumentItem : public PartItem
{
public:
    XdeDocumentItem(const Handle_TDocStd_Document& doc);

    const Handle_TDocStd_Document& cafDoc() const;
    const Handle_XCAFDoc_ShapeTool& shapeTool() const;
    const Handle_XCAFDoc_ColorTool& colorTool() const;

    TopoDS_Shape shape(const TDF_Label& lbl) const;
    QString findLabelName(const TDF_Label& lbl) const;

    TDF_LabelSequence topLevelFreeShapeLabels() const;

    bool isShape(const TDF_Label& lbl) const;
    bool isShapeAssembly(const TDF_Label& lbl) const;
    bool isShapeReference(const TDF_Label& lbl) const;
    bool isShapeSimple(const TDF_Label& lbl) const;
    bool isShapeComponent(const TDF_Label& lbl) const;
    bool isShapeCompound(const TDF_Label& lbl) const;
    bool isShapeSub(const TDF_Label& lbl) const;

    bool hasShapeColor(const TDF_Label& lbl) const;
    Quantity_Color shapeColor(const TDF_Label& lbl) const;

    TopLoc_Location shapeReferenceLocation(const TDF_Label& lbl) const;

    static const char* type;
    const char* dynType() const override;

private:
    Handle_TDocStd_Document m_cafDoc;
    Handle_XCAFDoc_ShapeTool m_shapeTool;
    Handle_XCAFDoc_ColorTool m_colorTool;
};

} // namespace Mayo
