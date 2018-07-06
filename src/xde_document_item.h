/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_item.h"
#include "libtree.h"
#include "quantity.h"
#include <TDF_ChildIterator.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <QtCore/QCoreApplication>

#include <vector>

namespace Mayo {

class XdeDocumentItem : public PartItem {
    Q_DECLARE_TR_FUNCTIONS(XdeDocumentItem)
public:
    using AssemblyNodeId = TreeNodeId;

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

    void rebuildAssemblyTree();
    const Tree<TDF_Label>& assemblyTree() const;

    template<typename LABEL_CONTAINER = std::vector<TDF_Label>>
    LABEL_CONTAINER topLevelFreeShapes() const;

    template<typename LABEL_CONTAINER = std::vector<TDF_Label>>
    LABEL_CONTAINER shapeComponents(const TDF_Label& lbl) const;

    template<typename LABEL_CONTAINER = std::vector<TDF_Label>>
    LABEL_CONTAINER shapeSubs(const TDF_Label& lbl) const;

    TopoDS_Shape shape(const TDF_Label& lbl) const;
    QString findLabelName(const TDF_Label& lbl) const;
    QString findLabelName(AssemblyNodeId nodeId) const;

    bool isShape(const TDF_Label& lbl) const;
    bool isShapeFree(const TDF_Label& lbl) const;
    bool isShapeAssembly(const TDF_Label& lbl) const;
    bool isShapeReference(const TDF_Label& lbl) const;
    bool isShapeSimple(const TDF_Label& lbl) const;
    bool isShapeComponent(const TDF_Label& lbl) const;
    bool isShapeCompound(const TDF_Label& lbl) const;
    bool isShapeSub(const TDF_Label& lbl) const;

    bool hasShapeColor(const TDF_Label& lbl) const;
    Quantity_Color shapeColor(const TDF_Label& lbl) const;

    TopLoc_Location shapeAbsoluteLocation(AssemblyNodeId nodeId) const;
    TopLoc_Location shapeReferenceLocation(const TDF_Label& lbl) const;
    TDF_Label shapeReferred(const TDF_Label& lbl) const;

    ValidationProperties validationProperties(const TDF_Label& lbl) const;
    std::vector<HandleProperty> shapeProperties(
            const TDF_Label& label,
            ShapePropertiesOption opt = ShapePropertiesOption::None) const;

    static const char TypeName[];
    const char* dynTypeName() const override;

private:
    template<typename LABEL_CONTAINER, typename PREDICATE>
    static void getDirectChildren(
            const TDF_Label& lbl, LABEL_CONTAINER* cnter, PREDICATE pred)
    {
        for (TDF_ChildIterator it(lbl); it.More(); it.Next()) {
            const TDF_Label child = it.Value();
            if (pred(child))
                cnter->push_back(child);
        }
    }

    void deepBuildAssemblyTree(
            AssemblyNodeId parentNode, const TDF_Label& label);

    Handle_TDocStd_Document m_cafDoc;
    Handle_XCAFDoc_ShapeTool m_shapeTool;
    Handle_XCAFDoc_ColorTool m_colorTool;
    Tree<TDF_Label> m_asmTree;
};

struct XdeAssemblyNode {
    XdeAssemblyNode() = default;
    XdeAssemblyNode(XdeDocumentItem* docItem,
                    XdeDocumentItem::AssemblyNodeId nodeId);
    static const XdeAssemblyNode& null();
    bool isValid() const;
    const TDF_Label& label() const;
    XdeDocumentItem* ownerDocItem;
    XdeDocumentItem::AssemblyNodeId nodeId;
};


// --
// -- Implementation
// --

template<typename LABEL_CONTAINER>
LABEL_CONTAINER XdeDocumentItem::topLevelFreeShapes() const {
    LABEL_CONTAINER cnter;
    const TDF_Label lbl = this->shapeTool()->Label();
    getDirectChildren(lbl, &cnter, [=](const TDF_Label& child) {
        return this->isShape(child) && this->isShapeFree(child);
    });
    return cnter;
}

template<typename LABEL_CONTAINER>
LABEL_CONTAINER XdeDocumentItem::shapeComponents(const TDF_Label& lbl) const {
    LABEL_CONTAINER cnter;
    if (this->isShapeAssembly(lbl)) {
        getDirectChildren(lbl, &cnter, [=](const TDF_Label& child) {
            return this->isShapeComponent(child);
        });
    }
    return cnter;
}

template<typename LABEL_CONTAINER>
LABEL_CONTAINER XdeDocumentItem::shapeSubs(const TDF_Label& lbl) const {
    LABEL_CONTAINER cnter;
    getDirectChildren(lbl, &cnter, [=](const TDF_Label& child) {
        return this->isShapeSub(child);
    });
    return cnter;
}

} // namespace Mayo
