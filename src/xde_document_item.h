/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
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

#include <memory>
#include <vector>

namespace Mayo {

class XdeShapePropertyOwner;

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

    TDF_LabelSequence topLevelFreeShapes() const;
    static TDF_LabelSequence shapeComponents(const TDF_Label& lbl);
    static TDF_LabelSequence shapeSubs(const TDF_Label& lbl);

    static QString findLabelName(const TDF_Label& lbl);
    QString findLabelName(AssemblyNodeId nodeId) const;
    static void setLabelName(const TDF_Label& lbl, const QString& name);
    void setLabelName(AssemblyNodeId nodeId, const QString& name);

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

    TopLoc_Location shapeAbsoluteLocation(AssemblyNodeId nodeId) const;
    static TopLoc_Location shapeReferenceLocation(const TDF_Label& lbl);
    static TDF_Label shapeReferred(const TDF_Label& lbl);

    static ValidationProperties validationProperties(const TDF_Label& lbl);
    std::unique_ptr<XdeShapePropertyOwner> shapeProperties(
            const TDF_Label& label,
            ShapePropertiesOption opt = ShapePropertiesOption::None) const;

    static const char TypeName[];
    const char* dynTypeName() const override;

private:
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

} // namespace Mayo
