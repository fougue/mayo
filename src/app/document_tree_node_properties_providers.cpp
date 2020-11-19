/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_tree_node_properties_providers.h"

#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/document_tree_node.h"
#include "../base/meta_enum.h"
#include "../base/string_utils.h"
#include "../base/xcaf.h"

#include <TDataXtd_Triangulation.hxx>

namespace Mayo {

class XCaf_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroupSignals {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::XCaf_DocumentTreeNodeProperties)
public:
    Properties(const DocumentTreeNode& treeNode)
        : m_propertyName(this, textId("Name")),
          m_propertyShapeType(this, textId("Shape")),
          m_propertyXdeShapeKind(this, textId("XdeShape")),
          m_propertyColor(this, textId("Color")),
          m_propertyReferenceLocation(this, textId("Location")),
          m_propertyValidationCentroid(this, textId("Centroid")),
          m_propertyValidationArea(this, textId("Area")),
          m_propertyValidationVolume(this, textId("Volume")),
          m_propertyReferredName(this, textId("ProductName")),
          m_propertyReferredColor(this, textId("ProductColor")),
          m_propertyReferredValidationCentroid(this, textId("ProductCentroid")),
          m_propertyReferredValidationArea(this, textId("ProductArea")),
          m_propertyReferredValidationVolume(this, textId("ProductVolume")),
          m_label(treeNode.label())
    {
        const TDF_Label& label = m_label;
        const XCaf& xcaf = treeNode.document()->xcaf();

        // Name
        m_propertyName.setValue(CafUtils::labelAttrStdName(label));

        // Shape type
        const TopAbs_ShapeEnum shapeType = XCAFDoc_ShapeTool::GetShape(label).ShapeType();
        m_propertyShapeType.setValue(MetaEnum::nameWithoutPrefix(shapeType, "TopAbs_").data());

        // XDE shape kind
        QStringList listXdeShapeKind;
        if (XCaf::isShapeAssembly(label))
            listXdeShapeKind.push_back(textId("Assembly").tr());

        if (XCaf::isShapeReference(label))
            listXdeShapeKind.push_back(textId("Reference").tr());

        if (XCaf::isShapeComponent(label))
            listXdeShapeKind.push_back(textId("Component").tr());

        if (XCaf::isShapeCompound(label))
            listXdeShapeKind.push_back(textId("Compound").tr());

        if (XCaf::isShapeSimple(label))
            listXdeShapeKind.push_back(textId("Simple").tr());

        if (XCaf::isShapeSub(label))
            listXdeShapeKind.push_back(textId("Sub").tr());

        m_propertyXdeShapeKind.setValue(listXdeShapeKind.join('+'));

        // Reference location
        if (XCaf::isShapeReference(label)) {
            const TopLoc_Location loc = XCaf::shapeReferenceLocation(label);
            m_propertyReferenceLocation.setValue(loc.Transformation());
        }
        else {
            this->removeProperty(&m_propertyReferenceLocation);
        }

        // Color
        if (xcaf.hasShapeColor(label))
            m_propertyColor.setValue(xcaf.shapeColor(label));
        else
            this->removeProperty(&m_propertyColor);

        // Validation properties
        {
            auto validProps = XCaf::validationProperties(label);
            m_propertyValidationCentroid.setValue(validProps.centroid);
            if (!validProps.hasCentroid)
                this->removeProperty(&m_propertyValidationCentroid);

            m_propertyValidationArea.setQuantity(validProps.area);
            if (!validProps.hasArea)
                this->removeProperty(&m_propertyValidationArea);

            m_propertyValidationVolume.setQuantity(validProps.volume);
            if (!validProps.hasVolume)
                this->removeProperty(&m_propertyValidationVolume);
        }

        // Referred entity's properties
        if (XCaf::isShapeReference(label)) {
            m_labelReferred = XCaf::shapeReferred(label);
            m_propertyReferredName.setValue(CafUtils::labelAttrStdName(m_labelReferred));
            auto validProps = XCaf::validationProperties(m_labelReferred);
            m_propertyReferredValidationCentroid.setValue(validProps.centroid);
            if (!validProps.hasCentroid)
                this->removeProperty(&m_propertyReferredValidationCentroid);

            m_propertyReferredValidationArea.setQuantity(validProps.area);
            if (!validProps.hasArea)
                this->removeProperty(&m_propertyReferredValidationArea);

            m_propertyReferredValidationVolume.setQuantity(validProps.volume);
            if (!validProps.hasVolume)
                this->removeProperty(&m_propertyReferredValidationVolume);

            if (xcaf.hasShapeColor(m_labelReferred))
                m_propertyReferredColor.setValue(xcaf.shapeColor(m_labelReferred));
            else
                this->removeProperty(&m_propertyReferredColor);
        }
        else {
            this->removeProperty(&m_propertyReferredName);
            this->removeProperty(&m_propertyReferredValidationCentroid);
            this->removeProperty(&m_propertyReferredValidationArea);
            this->removeProperty(&m_propertyReferredValidationVolume);
            this->removeProperty(&m_propertyReferredColor);
        }

        for (Property* prop : this->properties())
            prop->setUserReadOnly(true);

        m_propertyName.setUserReadOnly(false);
        m_propertyReferredName.setUserReadOnly(false);
    }

    void onPropertyChanged(Property* prop) override
    {
        if (prop == &m_propertyName)
            CafUtils::setLabelAttrStdName(m_label, m_propertyName.value());
        else if (prop == &m_propertyReferredName)
            CafUtils::setLabelAttrStdName(m_labelReferred, m_propertyReferredName.value());

        PropertyGroupSignals::onPropertyChanged(prop);
    }

    PropertyQString m_propertyName;
    PropertyQString m_propertyShapeType;
    PropertyQString m_propertyXdeShapeKind;
    PropertyOccColor m_propertyColor;
    PropertyOccTrsf m_propertyReferenceLocation;
    PropertyOccPnt m_propertyValidationCentroid;
    PropertyArea m_propertyValidationArea;
    PropertyVolume m_propertyValidationVolume;

    PropertyQString m_propertyReferredName;
    PropertyOccColor m_propertyReferredColor;
    PropertyOccPnt m_propertyReferredValidationCentroid;
    PropertyArea m_propertyReferredValidationArea;
    PropertyVolume m_propertyReferredValidationVolume;

    TDF_Label m_label;
    TDF_Label m_labelReferred;
};

bool XCaf_DocumentTreeNodePropertiesProvider::supports(const DocumentTreeNode& treeNode) const
{
    return XCaf::isShape(treeNode.label());
}

std::unique_ptr<PropertyGroupSignals>
XCaf_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

class Mesh_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroupSignals {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Mesh_DocumentTreeNodeProperties)
public:
        Properties(const DocumentTreeNode& treeNode)
      : m_propertyNodeCount(this, textId("NodeCount")),
        m_propertyTriangleCount(this, textId("TriangleCount"))
    {
        auto attrTriangulation = CafUtils::findAttribute<TDataXtd_Triangulation>(treeNode.label());
        Handle_Poly_Triangulation polyTri;
        if (!attrTriangulation.IsNull())
            polyTri = attrTriangulation->Get();

        m_propertyNodeCount.setValue(!polyTri.IsNull() ? polyTri->NbNodes() : 0);
        m_propertyTriangleCount.setValue(!polyTri.IsNull() ? polyTri->NbTriangles() : 0);
        m_propertyNodeCount.setUserReadOnly(true);
        m_propertyTriangleCount.setUserReadOnly(true);
    }

    PropertyInt m_propertyNodeCount; // Read-only
    PropertyInt m_propertyTriangleCount; // Read-only
};

bool Mesh_DocumentTreeNodePropertiesProvider::supports(const DocumentTreeNode& treeNode) const
{
    return CafUtils::hasAttribute<TDataXtd_Triangulation>(treeNode.label());
}

std::unique_ptr<PropertyGroupSignals>
Mesh_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

} // namespace Mayo
