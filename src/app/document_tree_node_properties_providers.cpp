/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_tree_node_properties_providers.h"

#include "../base/caf_utils.h"
#include "../base/label_data.h"
#include "../base/triangulation_annex_data.h"
#include "../base/document.h"
#include "../base/document_tree_node.h"
#include "../base/mesh_access.h"
#include "../base/mesh_utils.h"
#include "../base/meta_enum.h"
#include "../base/point_cloud_data.h"
#include "../base/xcaf.h"
#include "../graphics/graphics_mesh_object_driver.h"
#include "../graphics/graphics_point_cloud_object_driver.h"
#include "../graphics/graphics_shape_object_driver.h"
#include "../qtcommon/qstring_conv.h"

#include <Bnd_Box.hxx>
#include <TDataStd_Name.hxx>
#include <QtCore/QStringList>

namespace Mayo {

class XCaf_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroupSignals {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::XCaf_DocumentTreeNodeProperties)
public:
    Properties(const DocumentTreeNode& treeNode)
        : m_label(treeNode.label())
    {
        const TDF_Label& label = m_label;
        const XCaf& xcaf = treeNode.document()->xcaf();

        // Name
        m_propertyName.setValue(to_stdString(CafUtils::labelAttrStdName(label)));

        // Shape type
        const TopAbs_ShapeEnum shapeType = XCAFDoc_ShapeTool::GetShape(label).ShapeType();
        m_propertyShapeType.setValue(MetaEnum::nameWithoutPrefix(shapeType, "TopAbs_").data());

        // XDE shape kind
        {
            auto fnTrQString = [](const char* str){ return to_QString(textIdTr(str)); };
            QStringList listXdeShapeKind;
            if (XCaf::isShapeAssembly(label))
                listXdeShapeKind.push_back(fnTrQString("Assembly"));

            if (XCaf::isShapeReference(label))
                listXdeShapeKind.push_back(fnTrQString("Reference"));

            if (XCaf::isShapeComponent(label))
                listXdeShapeKind.push_back(fnTrQString("Component"));

            if (XCaf::isShapeCompound(label))
                listXdeShapeKind.push_back(fnTrQString("Compound"));

            if (XCaf::isShapeSimple(label))
                listXdeShapeKind.push_back(fnTrQString("Simple"));

            if (XCaf::isShapeSub(label))
                listXdeShapeKind.push_back(fnTrQString("Sub"));

            m_propertyXdeShapeKind.setValue(to_stdString(listXdeShapeKind.join('+')));
        }

        // XDE layers
        {
            TDF_LabelSequence seqLayerLabel = xcaf.layers(label);
            if (XCaf::isShapeReference(label)) {
                TDF_LabelSequence seqLayerLabelProduct = xcaf.layers(XCaf::shapeReferred(label));
                seqLayerLabel.Append(seqLayerLabelProduct);
            }

            QStringList listLayerName;
            for (const TDF_Label& layerLabel : seqLayerLabel)
                listLayerName.push_back(to_QString(xcaf.layerName(layerLabel)));

            if (!seqLayerLabel.IsEmpty())
                m_propertyXdeLayer.setValue(to_stdString(listLayerName.join(", ")));
            else
                this->removeProperty(&m_propertyXdeLayer);
        }

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

        // Material
        {
            const TDF_Label labelPart = XCaf::isShapeReference(label) ? XCaf::shapeReferred(label) : label;
            const OccHandle<XCAFDoc_Material> material = XCaf::shapeMaterial(labelPart);
            if (material) {
                m_propertyMaterialDensity.setQuantity(XCaf::shapeMaterialDensity(material));
                m_propertyMaterialName.setValue(to_stdString(material->GetName()));
            }
            else {
                this->removeProperty(&m_propertyMaterialDensity);
                this->removeProperty(&m_propertyMaterialName);
            }
        }

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
            m_propertyReferredName.setValue(to_stdString(CafUtils::labelAttrStdName(m_labelReferred)));
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
            TDataStd_Name::Set(m_label, to_OccExtString(m_propertyName.value()));
        else if (prop == &m_propertyReferredName)
            TDataStd_Name::Set(m_labelReferred, to_OccExtString(m_propertyReferredName.value()));

        PropertyGroupSignals::onPropertyChanged(prop);
    }

    PropertyString m_propertyName{ this, textId("Name") };
    PropertyString m_propertyShapeType{ this, textId("Shape") };
    PropertyString m_propertyXdeShapeKind{ this, textId("XdeShape") };
    PropertyString m_propertyXdeLayer{ this, textId("XdeLayer") };
    PropertyOccColor m_propertyColor{ this, textId("Color") };
    PropertyOccTrsf m_propertyReferenceLocation{ this, textId("Location") };
    PropertyOccPnt m_propertyValidationCentroid{ this, textId("Centroid") };
    PropertyArea m_propertyValidationArea{ this, textId("Area") };
    PropertyVolume m_propertyValidationVolume{ this, textId("Volume") };
    PropertyDensity m_propertyMaterialDensity{ this, textId("MaterialDensity") };
    PropertyString m_propertyMaterialName{ this, textId("MaterialName") };

    PropertyString m_propertyReferredName{ this, textId("ProductName") };
    PropertyOccColor m_propertyReferredColor{ this, textId("ProductColor") };
    PropertyOccPnt m_propertyReferredValidationCentroid{ this, textId("ProductCentroid") };
    PropertyArea m_propertyReferredValidationArea{ this, textId("ProductArea") };
    PropertyVolume m_propertyReferredValidationVolume{ this, textId("ProductVolume") };

    TDF_Label m_label;
    TDF_Label m_labelReferred;
};

bool XCaf_DocumentTreeNodePropertiesProvider::supports(const DocumentTreeNode& treeNode) const
{
    return GraphicsShapeObjectDriver::shapeSupportStatus(treeNode.label()) == GraphicsObjectDriver::Support::Complete;
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
    {
        OccHandle<Poly_Triangulation> mesh;
        IMeshAccess_visitMeshes(treeNode, [&](const IMeshAccess& access) {
            mesh = access.triangulation();
        });

        m_propertyNodeCount.setValue(!mesh.IsNull() ? mesh->NbNodes() : 0);
        m_propertyTriangleCount.setValue(!mesh.IsNull() ? mesh->NbTriangles() : 0);
        m_propertyArea.setQuantity(MeshUtils::triangulationArea(mesh) * Quantity_SquareMillimeter);
        m_propertyVolume.setQuantity(MeshUtils::triangulationVolume(mesh) * Quantity_CubicMillimeter);
        for (Property* property : this->properties())
            property->setUserReadOnly(true);
    }

    PropertyInt m_propertyNodeCount{ this, textId("NodeCount") };
    PropertyInt m_propertyTriangleCount{ this, textId("TriangleCount") };
    PropertyArea m_propertyArea{ this, textId("Area") };
    PropertyVolume m_propertyVolume{ this, textId("Volume") };
};

bool Mesh_DocumentTreeNodePropertiesProvider::supports(const DocumentTreeNode& treeNode) const
{
    return GraphicsMeshObjectDriver::meshSupportStatus(treeNode.label()) == GraphicsObjectDriver::Support::Complete;
}

std::unique_ptr<PropertyGroupSignals>
Mesh_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

class PointCloud_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroupSignals {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::PointCloud_DocumentTreeNodeProperties)
public:
    Properties(const DocumentTreeNode& treeNode)
    {
        auto attrPointCloudData = CafUtils::findAttribute<PointCloudData>(treeNode.label());

        const bool hasAttrData = !attrPointCloudData.IsNull() && !attrPointCloudData->points().IsNull();
        m_propertyPointCount.setValue(hasAttrData ? attrPointCloudData->points()->VertexNumber() : 0);
        m_propertyHasColors.setValue(hasAttrData ? attrPointCloudData->points()->HasVertexColors() : false);
        if (hasAttrData) {
            Bnd_Box bndBox;
            const int pntCount = attrPointCloudData->points()->VertexNumber();
            for (int i = 1; i <= pntCount; ++i)
                bndBox.Add(attrPointCloudData->points()->Vertice(i));

            m_propertyCornerMin.setValue(bndBox.CornerMin());
            m_propertyCornerMax.setValue(bndBox.CornerMax());
        }

        for (Property* property : this->properties())
            property->setUserReadOnly(true);
    }

    PropertyInt m_propertyPointCount{ this, textId("PointCount") };
    PropertyBool m_propertyHasColors{ this, textId("HasColors") };
    PropertyOccPnt m_propertyCornerMin{ this, textId("CornerMin") };
    PropertyOccPnt m_propertyCornerMax{ this, textId("CornerMax") };
};

bool PointCloud_DocumentTreeNodePropertiesProvider::supports(const DocumentTreeNode& treeNode) const
{
    return GraphicsPointCloudObjectDriver::pointCloudSupportStatus(treeNode.label()) == GraphicsObjectDriver::Support::Complete;
}

std::unique_ptr<PropertyGroupSignals>
PointCloud_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

} // namespace Mayo
