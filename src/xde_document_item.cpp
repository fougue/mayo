#include "xde_document_item.h"

#include "caf_utils.h"
#include "string_utils.h"

#include <Standard_GUID.hxx>
#include <TDF_AttributeIterator.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Volume.hxx>

namespace Mayo {

namespace Internal {

static void addValidationProperties(
        const XdeDocumentItem::ValidationProperties& validationProps,
        std::vector<HandleProperty>* ptrVecHndProp,
        const QString& nameFormat = QStringLiteral("%1"))
{
    PropertyOwner* propOwner = nullptr;
    const HandleProperty::Storage hndStorage = HandleProperty::Owner;
    if (validationProps.hasCentroid) {
        auto propCentroid = new PropertyOccPnt(
                    propOwner, nameFormat.arg(XdeDocumentItem::tr("Centroid")));
        propCentroid->setValue(validationProps.centroid);
        ptrVecHndProp->emplace_back(propCentroid, hndStorage);
    }
    if (validationProps.hasArea) {
        auto propArea = new PropertyArea(
                    propOwner, nameFormat.arg(XdeDocumentItem::tr("Area")));
        propArea->setQuantity(validationProps.area);
        ptrVecHndProp->emplace_back(propArea, hndStorage);
    }
    if (validationProps.hasVolume) {
        auto propVolume = new PropertyVolume(
                    propOwner, nameFormat.arg(XdeDocumentItem::tr("Volume")));
        propVolume->setQuantity(validationProps.volume);
        ptrVecHndProp->emplace_back(propVolume, hndStorage);
    }
}

} // namespace Internal

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

XdeDocumentItem::ValidationProperties XdeDocumentItem::validationProperties(
        const TDF_Label &lbl) const
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

std::vector<HandleProperty> XdeDocumentItem::shapeProperties(
        const TDF_Label& label, ShapePropertiesOption opt) const
{
    std::vector<HandleProperty> vecHndProp;
    const auto hndStorage = HandleProperty::Owner;

    auto propShapeType = new PropertyQString(nullptr, tr("Shape"));
    const TopAbs_ShapeEnum shapeType = this->shape(label).ShapeType();
    propShapeType->setValue(
                QString(StringUtils::rawText(shapeType)).remove("TopAbs_"));
    vecHndProp.emplace_back(propShapeType, hndStorage);

    QStringList listXdeShapeKind;
    if (this->isShapeAssembly(label))
        listXdeShapeKind.push_back(tr("Assembly"));
    if (this->isShapeReference(label))
        listXdeShapeKind.push_back(tr("Reference"));
    if (this->isShapeComponent(label))
        listXdeShapeKind.push_back(tr("Component"));
    if (this->isShapeCompound(label))
        listXdeShapeKind.push_back(tr("Compound"));
    if (this->isShapeSimple(label))
        listXdeShapeKind.push_back(tr("Simple"));
    if (this->isShapeSub(label))
        listXdeShapeKind.push_back(tr("Sub"));
    auto propXdeShapeKind = new PropertyQString(nullptr, tr("XDE shape"));
    propXdeShapeKind->setValue(listXdeShapeKind.join('+'));
    vecHndProp.emplace_back(propXdeShapeKind, hndStorage);

    if (this->isShapeReference(label)) {
        const TopLoc_Location loc = this->shapeReferenceLocation(label);
        auto propLoc = new PropertyOccTrsf(nullptr, tr("Location"));
        propLoc->setValue(loc.Transformation());
        vecHndProp.emplace_back(propLoc, hndStorage);
    }
    Internal::addValidationProperties(
                this->validationProperties(label), &vecHndProp);
    if (this->hasShapeColor(label)) {
        auto propColor = new PropertyOccColor(nullptr, tr("Color"));
        propColor->setValue(this->shapeColor(label));
        vecHndProp.emplace_back(propColor, hndStorage);
    }

    if (this->isShapeReference(label)
            && opt == ShapePropertiesOption::MergeReferred)
    {
        const TDF_Label referredLabel = this->shapeReferred(label);
        Internal::addValidationProperties(
                    this->validationProperties(referredLabel),
                    &vecHndProp,
                    tr("[Referred]%1"));
        if (this->hasShapeColor(referredLabel)) {
            auto propColor = new PropertyOccColor(nullptr, tr("[Referred]Color"));
            propColor->setValue(this->shapeColor(referredLabel));
            vecHndProp.emplace_back(propColor, hndStorage);
        }
    }
    return vecHndProp;
}

XdeDocumentItem::Label::Label(XdeDocumentItem *docItem, const TDF_Label &lbl)
    : xdeDocumentItem(docItem),
      label(lbl)
{}

const XdeDocumentItem::Label &XdeDocumentItem::Label::null()
{
    static const Label lbl = { nullptr, TDF_Label() };
    return lbl;
}

} // namespace Mayo
