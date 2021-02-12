/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "xcaf.h"

#include <TDocStd_Document.hxx>
#include <TDF_AttributeIterator.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Volume.hxx>

namespace Mayo {

bool XCaf::isNull() const
{
    Handle_TDocStd_Document doc = TDocStd_Document::Get(m_labelMain);
    if (!doc.IsNull()) {
        if (XCAFDoc_DocumentTool::IsXCAFDocument(doc))
            return false;
    }

    return true;
}

Handle_XCAFDoc_ShapeTool XCaf::shapeTool() const
{
    return XCAFDoc_DocumentTool::ShapeTool(m_labelMain);
}

Handle_XCAFDoc_ColorTool XCaf::colorTool() const
{
    return XCAFDoc_DocumentTool::ColorTool(m_labelMain);
}

#if OCC_VERSION_HEX >= 0x070500
Handle_XCAFDoc_VisMaterialTool XCaf::visMaterialTool() const
{
    return XCAFDoc_DocumentTool::VisMaterialTool(m_labelMain);
}
#endif

TDF_LabelSequence XCaf::topLevelFreeShapes() const
{
    TDF_LabelSequence seq;
    Handle_XCAFDoc_ShapeTool tool = this->shapeTool();
    if (tool)
        tool->GetFreeShapes(seq);

    return seq;
}

TDF_LabelSequence XCaf::shapeComponents(const TDF_Label& lbl)
{
    TDF_LabelSequence seq;
    XCAFDoc_ShapeTool::GetComponents(lbl, seq);
    return seq;
}

TDF_LabelSequence XCaf::shapeSubs(const TDF_Label& lbl)
{
    TDF_LabelSequence seq;
    XCAFDoc_ShapeTool::GetSubShapes(lbl, seq);
    return seq;
}

bool XCaf::isShape(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsShape(lbl);
}

bool XCaf::isShapeFree(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsFree(lbl);
}

TopoDS_Shape XCaf::shape(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::GetShape(lbl);
}

//QString XCaf::findLabelName(const TDF_Label& lbl)
//{
//    QString name = CafUtils::labelAttrStdName(lbl);
//    if (name.isEmpty()) {
//        if (XCaf::isShape(lbl)) {
//            const TopoDS_Shape shape = XCaf::shape(lbl);
//            switch (shape.ShapeType()) {
//            case TopAbs_COMPOUND: name = "Compound"; break;
//            case TopAbs_COMPSOLID: name = "CompSolid"; break;
//            case TopAbs_SOLID: name = "Solid"; break;
//            case TopAbs_SHELL: name = "Shell"; break;
//            case TopAbs_FACE: name = "Face"; break;
//            case TopAbs_WIRE: name = "Wire"; break;
//            case TopAbs_EDGE: name = "Edge"; break;
//            case TopAbs_VERTEX: name = "Vertex"; break;
//            case TopAbs_SHAPE: name = "Shape"; break;
//            }
//            name = QString("%1 %2").arg(name).arg(lbl.Tag());
//        }
//        else {
//            name = QString("[[%1]]").arg(CafUtils::labelTag(lbl));
//        }
//    }

//    return name;
//}

//QString XCaf::findLabelName(TreeNodeId nodeId) const
//{
//    return XCaf::findLabelName(m_asmTree.nodeData(nodeId));
//}

//void XCaf::setLabelName(TreeNodeId nodeId, const QString& name)
//{
//    XCaf::setLabelName(m_asmTree.nodeData(nodeId), name);
//}

bool XCaf::isShapeAssembly(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsAssembly(lbl);
}

bool XCaf::isShapeReference(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsReference(lbl);
}

bool XCaf::isShapeSimple(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsSimpleShape(lbl);
}

bool XCaf::isShapeComponent(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsComponent(lbl);
}

bool XCaf::isShapeCompound(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsCompound(lbl);
}

bool XCaf::isShapeSub(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsSubShape(lbl);
}

bool XCaf::hasShapeColor(const TDF_Label& lbl) const
{
    Handle_XCAFDoc_ColorTool tool = this->colorTool();
    if (!tool)
        return false;

    return tool->IsSet(lbl, XCAFDoc_ColorGen)
            || tool->IsSet(lbl, XCAFDoc_ColorSurf)
            || tool->IsSet(lbl, XCAFDoc_ColorCurv);
}

Quantity_Color XCaf::shapeColor(const TDF_Label& lbl) const
{
    Handle_XCAFDoc_ColorTool tool = this->colorTool();
    Quantity_Color color = {};
    if (!tool)
        return color;

    if (tool->GetColor(lbl, XCAFDoc_ColorGen, color))
        return color;

    if (tool->GetColor(lbl, XCAFDoc_ColorSurf, color))
        return color;

    if (tool->GetColor(lbl, XCAFDoc_ColorCurv, color))
        return color;

    return color;
}

TopLoc_Location XCaf::shapeReferenceLocation(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::GetLocation(lbl);
}

TDF_Label XCaf::shapeReferred(const TDF_Label& lbl)
{
    TDF_Label referred;
    XCAFDoc_ShapeTool::GetReferredShape(lbl, referred);
    return referred;
}

TopLoc_Location XCaf::shapeAbsoluteLocation(TreeNodeId nodeId) const
{
    Expects(m_modelTree != nullptr);
    return XCaf::shapeAbsoluteLocation(*m_modelTree, nodeId);
}

TopLoc_Location XCaf::shapeAbsoluteLocation(const Tree<TDF_Label>& modelTree, TreeNodeId nodeId)
{
    TopLoc_Location absoluteLoc;
    TreeNodeId it = nodeId;
    while (it != 0) {
        const TDF_Label& nodeLabel = modelTree.nodeData(it);
        const TopLoc_Location nodeLoc = XCaf::shapeReferenceLocation(nodeLabel);
        absoluteLoc = nodeLoc * absoluteLoc;
        it = modelTree.nodeParent(it);
    }

    return absoluteLoc;
}

XCaf::ValidationProperties XCaf::validationProperties(const TDF_Label& lbl)
{
    ValidationProperties props = {};
    for (TDF_AttributeIterator it(lbl); it.More(); it.Next()) {
        const Handle_TDF_Attribute ptrAttr = it.Value();
        const Standard_GUID& attrId = ptrAttr->ID();
        if (&attrId == &XCAFDoc_Centroid::GetID()) {
            const auto& centroid = static_cast<const XCAFDoc_Centroid&>(*ptrAttr);
            props.hasCentroid = true;
            props.centroid = centroid.Get();
        }
        else if (&attrId == &XCAFDoc_Area::GetID()) {
            const auto& area = static_cast<const XCAFDoc_Area&>(*ptrAttr);
            props.hasArea = true;
            props.area = area.Get() * Quantity_SquaredMillimeter;
        }
        else if (&attrId == &XCAFDoc_Volume::GetID()) {
            const auto& volume = static_cast<const XCAFDoc_Volume&>(*ptrAttr);
            props.hasVolume = true;
            props.volume = volume.Get() * Quantity_CubicMillimeter;
        }

        if (props.hasCentroid && props.hasArea && props.hasVolume)
            break;
    }

    return props;
}

TreeNodeId XCaf::deepBuildAssemblyTree(TreeNodeId parentNode, const TDF_Label& label)
{
    Expects(m_modelTree != nullptr);

    const TreeNodeId node = m_modelTree->appendChild(parentNode, label);
    if (XCaf::isShapeAssembly(label)) {
        for (const TDF_Label& child : XCaf::shapeComponents(label))
            this->deepBuildAssemblyTree(node, child);
    }
    else if (XCaf::isShapeReference(label)) {
        const TDF_Label referred = XCaf::shapeReferred(label);
        this->deepBuildAssemblyTree(node, referred);
    }
#if 0
    else if (XCaf::isShapeSimple(label)) {
        for (const TDF_Label& child : XCaf::shapeSubs(label))
            this->deepBuildAssemblyTree(node, child);
    }
#endif

    return node;
}

} // namespace Mayo
