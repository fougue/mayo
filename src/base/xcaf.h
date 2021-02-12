/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "libtree.h"
#include "quantity.h"
#include <Quantity_Color.hxx>
#include <Standard_Version.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#if OCC_VERSION_HEX >= 0x070500
#  include <XCAFDoc_VisMaterialTool.hxx>
#endif

namespace Mayo {

// Closely related to Mayo::Document
class XCaf {
public:
    struct ValidationProperties {
        bool hasCentroid;
        bool hasArea;
        bool hasVolume;
        gp_Pnt centroid;
        QuantityArea area;
        QuantityVolume volume;
    };

    bool isNull() const;

    Handle_XCAFDoc_ShapeTool shapeTool() const;
    Handle_XCAFDoc_ColorTool colorTool() const;
#if OCC_VERSION_HEX >= 0x070500
    Handle_XCAFDoc_VisMaterialTool visMaterialTool() const;
#endif

    TDF_LabelSequence topLevelFreeShapes() const;
    static TDF_LabelSequence shapeComponents(const TDF_Label& lbl);
    static TDF_LabelSequence shapeSubs(const TDF_Label& lbl);

    static TopoDS_Shape shape(const TDF_Label& lbl);
    static bool isShape(const TDF_Label& lbl);
    static bool isShapeFree(const TDF_Label& lbl);
    static bool isShapeAssembly(const TDF_Label& lbl);
    static bool isShapeReference(const TDF_Label& lbl);
    static bool isShapeSimple(const TDF_Label& lbl);
    static bool isShapeComponent(const TDF_Label& lbl);
    static bool isShapeCompound(const TDF_Label& lbl);
    static bool isShapeSub(const TDF_Label& lbl);

    bool hasShapeColor(const TDF_Label& lbl) const;
    Quantity_Color shapeColor(const TDF_Label& lbl) const;

    TopLoc_Location shapeAbsoluteLocation(TreeNodeId nodeId) const;
    static TopLoc_Location shapeAbsoluteLocation(const Tree<TDF_Label>& modelTree, TreeNodeId nodeId);
    static TopLoc_Location shapeReferenceLocation(const TDF_Label& lbl);
    static TDF_Label shapeReferred(const TDF_Label& lbl);

    static ValidationProperties validationProperties(const TDF_Label& lbl);

private:
    XCaf() = default;

    TreeNodeId deepBuildAssemblyTree(TreeNodeId parentNode, const TDF_Label& label);
    void setLabelMain(const TDF_Label& labelMain) { m_labelMain = labelMain; }
    void setModelTree(Tree<TDF_Label>& modelTree) { m_modelTree = &modelTree; }

    friend class Document;
    TDF_Label m_labelMain;
    Tree<TDF_Label>* m_modelTree = nullptr;
};

} // namespace Mayo
