#include "xde_document_item.h"

#include "caf_utils.h"

#include <XCAFDoc_DocumentTool.hxx>

namespace Mayo {

XdeDocumentItem::XdeDocumentItem(const Handle_TDocStd_Document &doc)
    : m_cafDoc(doc),
      m_shapeTool(XCAFDoc_DocumentTool::ShapeTool(doc->Main())),
      m_colorTool(XCAFDoc_DocumentTool::ColorTool(doc->Main()))
{
}

const Handle_TDocStd_Document &XdeDocumentItem::cafDoc() const
{
    return m_cafDoc;
}

const Handle_XCAFDoc_ShapeTool &XdeDocumentItem::shapeTool() const
{
    return m_shapeTool;
}

const Handle_XCAFDoc_ColorTool &XdeDocumentItem::colorTool() const
{
    return m_colorTool;
}

bool XdeDocumentItem::isShape(const TDF_Label &lbl) const
{
    return m_shapeTool->IsShape(lbl);
}

TopoDS_Shape XdeDocumentItem::shape(const TDF_Label &lbl) const
{
    return m_shapeTool->GetShape(lbl);
}

QString XdeDocumentItem::findLabelName(const TDF_Label &lbl) const
{
    QString name = occ::CafUtils::labelAttrStdName(lbl);
    if (name.isEmpty()) {
        if (this->isShape(lbl)) {
            const TopoDS_Shape shape = this->shape(lbl);
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
            name = QString("[[%1]]").arg(occ::CafUtils::labelTag(lbl));
        }
    }
    return name;
}

TDF_LabelSequence XdeDocumentItem::topLevelFreeShapeLabels() const
{
    TDF_LabelSequence seq;
    m_shapeTool->GetFreeShapes(seq);
    return seq;
}

bool XdeDocumentItem::isShapeAssembly(const TDF_Label &lbl) const
{
    return m_shapeTool->IsAssembly(lbl);
}

bool XdeDocumentItem::isShapeReference(const TDF_Label &lbl) const
{
    return m_shapeTool->IsReference(lbl);
}

bool XdeDocumentItem::isShapeSimple(const TDF_Label &lbl) const
{
    return m_shapeTool->IsSimpleShape(lbl);
}

bool XdeDocumentItem::isShapeComponent(const TDF_Label &lbl) const
{
    return m_shapeTool->IsComponent(lbl);
}

bool XdeDocumentItem::isShapeCompound(const TDF_Label &lbl) const
{
    return m_shapeTool->IsCompound(lbl);
}

bool XdeDocumentItem::isShapeSub(const TDF_Label &lbl) const
{
    return m_shapeTool->IsSubShape(lbl);
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

TopLoc_Location XdeDocumentItem::shapeReferenceLocation(const TDF_Label &lbl) const
{
    return m_shapeTool->GetLocation(lbl);
}

TDF_Label XdeDocumentItem::shapeReferred(const TDF_Label &lbl) const
{
    TDF_Label referred;
    m_shapeTool->GetReferredShape(lbl, referred);
    return referred;
}

const char XdeDocumentItem::TypeName[] = "2a3efb26-cd32-432d-b95c-cdc64c3cf7d9";
const char *XdeDocumentItem::dynTypeName() const
{
    return XdeDocumentItem::TypeName;
}

} // namespace Mayo
