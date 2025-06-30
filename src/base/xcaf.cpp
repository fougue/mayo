/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "xcaf.h"
#include "caf_utils.h"
#include "math_utils.h"

#include <TDataStd_TreeNode.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_AttributeIterator.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Volume.hxx>
#include <set>

namespace Mayo {

bool XCaf::isNull() const
{
    OccHandle<TDocStd_Document> doc = TDocStd_Document::Get(m_labelMain);
    if (!doc.IsNull()) {
        if (XCAFDoc_DocumentTool::IsXCAFDocument(doc))
            return false;
    }

    return true;
}

OccHandle<XCAFDoc_ShapeTool> XCaf::shapeTool() const
{
    return XCAFDoc_DocumentTool::ShapeTool(m_labelMain);
}

OccHandle<XCAFDoc_LayerTool> XCaf::layerTool() const
{
    return XCAFDoc_DocumentTool::LayerTool(m_labelMain);
}

OccHandle<XCAFDoc_ColorTool> XCaf::colorTool() const
{
    return XCAFDoc_DocumentTool::ColorTool(m_labelMain);
}

OccHandle<XCAFDoc_MaterialTool> XCaf::materialTool() const
{
    return XCAFDoc_DocumentTool::MaterialTool(m_labelMain);
}

#if OCC_VERSION_HEX >= 0x070500
OccHandle<XCAFDoc_VisMaterialTool> XCaf::visMaterialTool() const
{
    return XCAFDoc_DocumentTool::VisMaterialTool(m_labelMain);
}
#endif

TDF_LabelSequence XCaf::topLevelFreeShapes() const
{
    TDF_LabelSequence seq;
    OccHandle<XCAFDoc_ShapeTool> tool = this->shapeTool();
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

void XCaf::setShape(const TDF_Label& label, const TopoDS_Shape& shape)
{
    this->shapeTool()->SetShape(label, shape);
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

bool XCaf::isShapeSubOf(const TDF_Label& lbl, const TopoDS_Shape& shape)
{
    return this->shapeTool()->IsSubShape(lbl, shape);
}

bool XCaf::hasShapeUsers(const TDF_Label& lbl)
{
    OccHandle<TDataStd_TreeNode> treeNode;
    if (!lbl.FindAttribute(XCAFDoc::ShapeRefGUID(), treeNode))
        return true;

    return treeNode && !treeNode->First().IsNull();
}

bool XCaf::hasShapeColor(const TDF_Label& lbl) const
{
    OccHandle<XCAFDoc_ColorTool> tool = this->colorTool();
    if (!tool)
        return false;

    return tool->IsSet(lbl, XCAFDoc_ColorGen)
           || tool->IsSet(lbl, XCAFDoc_ColorSurf)
           || tool->IsSet(lbl, XCAFDoc_ColorCurv)
        ;
}

Quantity_Color XCaf::shapeColor(const TDF_Label& lbl) const
{
    OccHandle<XCAFDoc_ColorTool> tool = this->colorTool();
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

TDF_Label XCaf::findShapeLabel(const TopoDS_Shape& shape, FindShapeLabelFlags flags) const
{
    TDF_Label label;
    const bool findInstance = (flags & FindShapeLabel_Instance) != 0;
    const bool findComponent = (flags & FindShapeLabel_Component) != 0;
    const bool findSubShape = (flags & FindShapeLabel_SubShape) != 0;
    if (this->shapeTool()->Search(shape, label, findInstance, findComponent, findSubShape))
        return label;

    return {};
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

TDF_LabelSequence XCaf::layers(const TDF_Label& lbl) const
{
    TDF_LabelSequence seq;
    auto tool = this->layerTool();
    if (tool)
        tool->GetLayers(lbl, seq);

    return seq;
}

TCollection_ExtendedString XCaf::layerName(const TDF_Label &lbl) const
{
    TCollection_ExtendedString name;
    auto tool = this->layerTool();
    if (tool)
        tool->GetLayer(lbl, name);

    return name;
}

TopLoc_Location XCaf::shapeAbsoluteLocation(TreeNodeId nodeId) const
{
    Expects(m_modelTree != nullptr);
    return XCaf::shapeAbsoluteLocation(*m_modelTree, nodeId);
}

TopLoc_Location XCaf::shapeAbsoluteLocation(const Tree<TDF_Label>& modelTree, TreeNodeId nodeId)
{
    TopLoc_Location absoluteLoc;
    for (TreeNodeId it = nodeId; it != 0; it = modelTree.nodeParent(it)) {
        const TDF_Label& nodeLabel = modelTree.nodeData(it);
        const TopLoc_Location nodeLoc = XCaf::shapeReferenceLocation(nodeLabel);
        absoluteLoc = nodeLoc * absoluteLoc;
    }

    return absoluteLoc;
}

QuantityDensity XCaf::shapeMaterialDensity(const TDF_Label& lbl)
{
    return XCaf::shapeMaterialDensity(XCaf::shapeMaterial(lbl));
}

QuantityDensity XCaf::shapeMaterialDensity(const OccHandle<XCAFDoc_Material>& material)
{
    const double density = material ? material->GetDensity() : 0.;
    return density * Quantity_GramPerCubicCentimeter;
}

OccHandle<XCAFDoc_Material> XCaf::shapeMaterial(const TDF_Label& lbl)
{
    OccHandle<TDataStd_TreeNode> node;
    if (!lbl.FindAttribute(XCAFDoc::MaterialRefGUID(), node) || !node->HasFather())
        return {};

    const TDF_Label labelMaterial = node->Father()->Label();
    return CafUtils::findAttribute<XCAFDoc_Material>(labelMaterial);
}

XCaf::ValidationProperties XCaf::validationProperties(const TDF_Label& lbl)
{
    ValidationProperties props = {};
    for (TDF_AttributeIterator it(lbl); it.More(); it.Next()) {
        const OccHandle<TDF_Attribute> ptrAttr = it.Value();
        const Standard_GUID& attrId = ptrAttr->ID();
        if (&attrId == &XCAFDoc_Centroid::GetID()) {
            const auto& centroid = static_cast<const XCAFDoc_Centroid&>(*ptrAttr);
            props.hasCentroid = true;
            props.centroid = centroid.Get();
        }
        else if (&attrId == &XCAFDoc_Area::GetID()) {
            const auto& area = static_cast<const XCAFDoc_Area&>(*ptrAttr);
            props.hasArea = true;
            props.area = area.Get() * Quantity_SquareMillimeter;
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

TDF_LabelSequence XCaf::diffTopLevelFreeShapes(const TDF_LabelSequence& seqOther) const
{
    const TDF_LabelSequence& seqBefore = seqOther;
    const TDF_LabelSequence seqAfter = this->topLevelFreeShapes();
    std::set<int> setBeforeTag;
    const TDF_Label firstBeforeLabel = !seqBefore.IsEmpty() ? seqBefore.First() : TDF_Label();
    for (const TDF_Label& label : seqBefore) {
        Expects(firstBeforeLabel.IsNull() || label.Depth() == firstBeforeLabel.Depth());
        setBeforeTag.insert(label.Tag());
    }

    TDF_LabelSequence seqDiff;
    for (const TDF_Label& label : seqAfter) {
        Expects(firstBeforeLabel.IsNull() || label.Depth() == firstBeforeLabel.Depth());
        if (setBeforeTag.find(label.Tag()) == setBeforeTag.cend())
            seqDiff.Append(label);
    }

    return seqDiff;
}

OccHandle<TDataStd_NamedData> XCaf::shapeUserDefinedAttributes(const TDF_Label& lbl) const
{
    if (lbl.IsNull())
        return {};

#if OCC_VERSION_HEX >= 0x070400
    return this->shapeTool()->GetNamedProperties(lbl, false/*!create*/);
#else
    return {};
#endif
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
