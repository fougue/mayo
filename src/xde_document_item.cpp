/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "xde_document_item.h"
#include "xde_shape_property_owner.h"
#include "caf_utils.h"
#include <fougtools/occtools/qt_utils.h>

#include <Standard_GUID.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_AttributeIterator.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Volume.hxx>

namespace Mayo {

XdeDocumentItem::XdeDocumentItem(const Handle_TDocStd_Document &doc)
    : m_cafDoc(doc),
      m_shapeTool(XCAFDoc_DocumentTool::ShapeTool(doc->Main())),
      m_colorTool(XCAFDoc_DocumentTool::ColorTool(doc->Main()))
{
    this->rebuildAssemblyTree();
}

const Handle_TDocStd_Document& XdeDocumentItem::cafDoc() const
{
    return m_cafDoc;
}

const Handle_XCAFDoc_ShapeTool& XdeDocumentItem::shapeTool() const
{
    return m_shapeTool;
}

const Handle_XCAFDoc_ColorTool& XdeDocumentItem::colorTool() const
{
    return m_colorTool;
}

void XdeDocumentItem::rebuildAssemblyTree()
{
    m_asmTree.clear();
    for (const TDF_Label& rootLabel : this->topLevelFreeShapes())
        this->deepBuildAssemblyTree(0, rootLabel);
}

const Tree<TDF_Label>& XdeDocumentItem::assemblyTree() const
{
    return m_asmTree;
}

TDF_LabelSequence XdeDocumentItem::topLevelFreeShapes() const
{
    TDF_LabelSequence seq;
    m_shapeTool->GetFreeShapes(seq);
    return seq;
}

TDF_LabelSequence XdeDocumentItem::shapeComponents(const TDF_Label& lbl)
{
    TDF_LabelSequence seq;
    XCAFDoc_ShapeTool::GetComponents(lbl, seq);
    return seq;
}

TDF_LabelSequence XdeDocumentItem::shapeSubs(const TDF_Label& lbl)
{
    TDF_LabelSequence seq;
    XCAFDoc_ShapeTool::GetSubShapes(lbl, seq);
    return seq;
}

bool XdeDocumentItem::isShape(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsShape(lbl);
}

bool XdeDocumentItem::isShapeFree(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsFree(lbl);
}

TopoDS_Shape XdeDocumentItem::shape(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::GetShape(lbl);
}

QString XdeDocumentItem::findLabelName(const TDF_Label& lbl)
{
    QString name = CafUtils::labelAttrStdName(lbl);
    if (name.isEmpty()) {
        if (XdeDocumentItem::isShape(lbl)) {
            const TopoDS_Shape shape = XdeDocumentItem::shape(lbl);
            switch (shape.ShapeType()) {
            case TopAbs_COMPOUND: name = "Compound"; break;
            case TopAbs_COMPSOLID: name = "CompSolid"; break;
            case TopAbs_SOLID: name = "Solid"; break;
            case TopAbs_SHELL: name = "Shell"; break;
            case TopAbs_FACE: name = "Face"; break;
            case TopAbs_WIRE: name = "Wire"; break;
            case TopAbs_EDGE: name = "Edge"; break;
            case TopAbs_VERTEX: name = "Vertex"; break;
            case TopAbs_SHAPE: name = "Shape"; break;
            }
            name = QString("%1 %2").arg(name).arg(lbl.Tag());
        }
        else {
            name = QString("[[%1]]").arg(CafUtils::labelTag(lbl));
        }
    }
    return name;
}

QString XdeDocumentItem::findLabelName(AssemblyNodeId nodeId) const
{
    return XdeDocumentItem::findLabelName(m_asmTree.nodeData(nodeId));
}

void XdeDocumentItem::setLabelName(const TDF_Label& lbl, const QString& name)
{
    TDataStd_Name::Set(lbl, occ::QtUtils::toOccExtendedString(name));
}

void XdeDocumentItem::setLabelName(AssemblyNodeId nodeId, const QString& name)
{
    XdeDocumentItem::setLabelName(m_asmTree.nodeData(nodeId), name);
}

bool XdeDocumentItem::isShapeAssembly(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsAssembly(lbl);
}

bool XdeDocumentItem::isShapeReference(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsReference(lbl);
}

bool XdeDocumentItem::isShapeSimple(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsSimpleShape(lbl);
}

bool XdeDocumentItem::isShapeComponent(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsComponent(lbl);
}

bool XdeDocumentItem::isShapeCompound(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsCompound(lbl);
}

bool XdeDocumentItem::isShapeSub(const TDF_Label& lbl)
{
    return XCAFDoc_ShapeTool::IsSubShape(lbl);
}

bool XdeDocumentItem::hasShapeColor(const TDF_Label &lbl) const
{
    return m_colorTool->IsSet(lbl, XCAFDoc_ColorGen)
            || m_colorTool->IsSet(lbl, XCAFDoc_ColorSurf)
            || m_colorTool->IsSet(lbl, XCAFDoc_ColorCurv);
}

Quantity_Color XdeDocumentItem::shapeColor(const TDF_Label &lbl) const
{
    Quantity_Color color;
    if (m_colorTool->GetColor(lbl, XCAFDoc_ColorGen, color))
        return color;
    if (m_colorTool->GetColor(lbl, XCAFDoc_ColorSurf, color))
        return color;
    if (m_colorTool->GetColor(lbl, XCAFDoc_ColorCurv, color))
        return color;
    return color;
}

TopLoc_Location XdeDocumentItem::shapeReferenceLocation(const TDF_Label &lbl)
{
    return XCAFDoc_ShapeTool::GetLocation(lbl);
}

TDF_Label XdeDocumentItem::shapeReferred(const TDF_Label &lbl)
{
    TDF_Label referred;
    XCAFDoc_ShapeTool::GetReferredShape(lbl, referred);
    return referred;
}

TopLoc_Location XdeDocumentItem::shapeAbsoluteLocation(AssemblyNodeId nodeId) const
{
    TopLoc_Location absoluteLoc;
    AssemblyNodeId it = nodeId;
    while (it != 0) {
        const TDF_Label& nodeLabel = m_asmTree.nodeData(it);
        const TopLoc_Location nodeLoc = XdeDocumentItem::shapeReferenceLocation(nodeLabel);
        absoluteLoc = nodeLoc * absoluteLoc;
        it = m_asmTree.nodeParent(it);
    }
    return absoluteLoc;
}

XdeDocumentItem::ValidationProperties XdeDocumentItem::validationProperties(
        const TDF_Label& lbl)
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

const char XdeDocumentItem::TypeName[] = "2a3efb26-cd32-432d-b95c-cdc64c3cf7d9";
const char *XdeDocumentItem::dynTypeName() const
{
    return XdeDocumentItem::TypeName;
}

void XdeDocumentItem::deepBuildAssemblyTree(
        AssemblyNodeId parentNode, const TDF_Label &label)
{
    const AssemblyNodeId node = m_asmTree.appendChild(parentNode, label);
    if (XdeDocumentItem::isShapeAssembly(label)) {
        for (const TDF_Label& child : XdeDocumentItem::shapeComponents(label))
            this->deepBuildAssemblyTree(node, child);
    }
    else if (XdeDocumentItem::isShapeSimple(label)) {
        for (const TDF_Label& child : XdeDocumentItem::shapeSubs(label))
            this->deepBuildAssemblyTree(node, child);
    }
    else if (XdeDocumentItem::isShapeReference(label)) {
        const TDF_Label referred = XdeDocumentItem::shapeReferred(label);
        this->deepBuildAssemblyTree(node, referred);
    }
}

std::unique_ptr<XdeShapePropertyOwner> XdeDocumentItem::shapeProperties(
        const TDF_Label& label, XdeDocumentItem::ShapePropertiesOption opt) const
{
    auto owner = new XdeShapePropertyOwner(this, label, opt);
    std::unique_ptr<XdeShapePropertyOwner> ptr(owner);
    return ptr;
}

XdeAssemblyNode::XdeAssemblyNode(
        XdeDocumentItem* docItem, XdeDocumentItem::AssemblyNodeId nde)
    : ownerDocItem(docItem),
      nodeId(nde)
{}

bool XdeAssemblyNode::isValid() const
{
    return this->ownerDocItem != nullptr && this->nodeId != 0;
}

const TDF_Label &XdeAssemblyNode::label() const
{
    static const TDF_Label nullLabel;
    return this->ownerDocItem != nullptr ?
                this->ownerDocItem->assemblyTree().nodeData(this->nodeId) :
                nullLabel;
}

const XdeAssemblyNode &XdeAssemblyNode::null()
{
    static const XdeAssemblyNode node = {};
    return node;
}

} // namespace Mayo
