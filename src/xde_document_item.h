/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_item.h"
#include "quantity.h"
#include <TDocStd_Document.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <QtCore/QCoreApplication>

namespace Mayo {

class XdeDocumentItem : public PartItem {
    Q_DECLARE_TR_FUNCTIONS(XdeDocumentItem)
public:
    struct Label {
        Label() = default;
        Label(XdeDocumentItem* docItem, const TDF_Label& lbl);
        static const Label& null();
        XdeDocumentItem* xdeDocumentItem;
        TDF_Label label;
    };

    struct ValidationProperties {
        bool hasCentroid;
        bool hasArea;
        bool hasVolume;
        gp_Pnt centroid;
        QuantityArea area;
        QuantityVolume volume;
    };

    enum class ShapePropertiesOption {
        None,
        MergeReferred
    };

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
    TDF_Label shapeReferred(const TDF_Label& lbl) const;

    ValidationProperties validationProperties(const TDF_Label& lbl) const;
    std::vector<HandleProperty> shapeProperties(
            const TDF_Label& label,
            ShapePropertiesOption opt = ShapePropertiesOption::None) const;

    static const char TypeName[];
    const char* dynTypeName() const override;

private:
    Handle_TDocStd_Document m_cafDoc;
    Handle_XCAFDoc_ShapeTool m_shapeTool;
    Handle_XCAFDoc_ColorTool m_colorTool;
};

} // namespace Mayo
