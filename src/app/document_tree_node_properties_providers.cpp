/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_tree_node_properties_providers.h"

#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/document_tree_node.h"
#include "../base/label_data.h"
#include "../base/mesh_access.h"
#include "../base/mesh_utils.h"
#include "../base/meta_enum.h"
#include "../base/point_cloud_data.h"
#include "../base/property_builtins.h"
#include "../base/string_conv.h"
#include "../base/xcaf.h"
#include "../graphics/graphics_mesh_object_driver.h"
#include "../graphics/graphics_point_cloud_object_driver.h"
#include "../graphics/graphics_shape_object_driver.h"
#include "../qtcommon/qstring_conv.h"

#include <Bnd_Box.hxx>
#include <TDataStd_Name.hxx>

#include <QtCore/QStringList>

#include <fmt/format.h>

namespace Mayo {

// NOTE
//   Using "enum XCafSubGroup : uint64_t" fools lupdate parser
static constexpr uint64_t GeneralCoreSubGroup = 1;
static constexpr uint64_t XCafSubGroup_Core = GeneralCoreSubGroup;
static constexpr uint64_t XCafSubGroup_Validation = 2;
static constexpr uint64_t XCafSubGroup_MetaData = 3;
static constexpr uint64_t XCafSubGroup_ProductMetaData = 4;

class XCaf_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::XCaf_DocumentTreeNodeProperties)
public:
    Properties(const DocumentTreeNode& treeNode);

    void onPropertyChanged(Property* prop) override;
    void addUdas(
        const OccHandle<TDataStd_NamedData>& data,
        std::vector<std::unique_ptr<Property>>& vecProperty
    );
    std::unique_ptr<Property> createProperty(
        const CafUtils::NamedDataKey& key,
        const TextId& name,
        const OccHandle<TDataStd_NamedData>& data
    );

    PropertyString m_propertyName{ this, textId("Name") };
    PropertyString m_propertyShapeType{ this, textId("Shape") };
    PropertyString m_propertyXdeShapeKind{ this, textId("XdeShape") };
    PropertyString m_propertyXdeLayer{ this, textId("XdeLayer") };
    PropertyOccColor m_propertyColor{ this, textId("Color") };
    PropertyOccTrsf m_propertyInstanceLocation{ this, textId("Location") };
    PropertyOccPnt m_propertyValidationCentroid{ this, textId("Centroid") };
    PropertyArea m_propertyValidationArea{ this, textId("Area") };
    PropertyVolume m_propertyValidationVolume{ this, textId("Volume") };
    PropertyDensity m_propertyMaterialDensity{ this, textId("MaterialDensity") };
    PropertyString m_propertyMaterialName{ this, textId("MaterialName") };

    PropertyString m_propertyProductName{ this, textId("ProductName") };
    PropertyOccColor m_propertyProductColor{ this, textId("ProductColor") };
    PropertyOccPnt m_propertyProductValidationCentroid{ this, textId("ProductCentroid") };
    PropertyArea m_propertyProductValidationArea{ this, textId("ProductArea") };
    PropertyVolume m_propertyProductValidationVolume{ this, textId("ProductVolume") };

    std::vector<std::unique_ptr<Property>> m_vecPropertyUda;
    std::vector<std::unique_ptr<Property>> m_vecPropertyProductUda;
    std::vector<std::string> m_textIdStringStorage;

    TDF_Label m_label;
    TDF_Label m_labelProduct;
};

XCaf_DocumentTreeNodePropertiesProvider::Properties::Properties(const DocumentTreeNode& treeNode)
    : m_label(treeNode.label())
{
    const TDF_Label& label = m_label;
    const XCaf& xcaf = treeNode.document()->xcaf();

    m_propertyName.setUserData(XCafSubGroup_Core);
    m_propertyShapeType.setUserData(XCafSubGroup_Core);
    m_propertyXdeShapeKind.setUserData(XCafSubGroup_Core);
    m_propertyXdeLayer.setUserData(XCafSubGroup_Core);
    m_propertyColor.setUserData(XCafSubGroup_Core);
    m_propertyInstanceLocation.setUserData(XCafSubGroup_Core);
    m_propertyValidationCentroid.setUserData(XCafSubGroup_Validation);
    m_propertyValidationArea.setUserData(XCafSubGroup_Validation);
    m_propertyValidationVolume.setUserData(XCafSubGroup_Validation);
    m_propertyMaterialDensity.setUserData(XCafSubGroup_Core);
    m_propertyMaterialName.setUserData(XCafSubGroup_Core);
    m_propertyProductName.setUserData(XCafSubGroup_Core);
    m_propertyProductColor.setUserData(XCafSubGroup_Core);
    m_propertyProductValidationCentroid.setUserData(XCafSubGroup_Validation);
    m_propertyProductValidationArea.setUserData(XCafSubGroup_Validation);
    m_propertyProductValidationVolume.setUserData(XCafSubGroup_Validation);
    m_propertyProductName.setUserData(XCafSubGroup_Core);

    // Name
    m_propertyName.setValue(to_stdString(CafUtils::labelAttrStdName(label)));

    // Shape type
    const TopAbs_ShapeEnum shapeType = XCAFDoc_ShapeTool::GetShape(label).ShapeType();
    m_propertyShapeType.setValue(MetaEnum::nameWithoutPrefix(shapeType, "TopAbs_").data());

    // XDE shape kind
    {
        QStringList listXdeShapeKind;
        if (XCaf::isShapeAssembly(label))
            listXdeShapeKind.push_back(to_QString(textIdTr("Assembly")));

        if (XCaf::isShapeReference(label))
            listXdeShapeKind.push_back(to_QString(textIdTr("Reference")));

        if (XCaf::isShapeComponent(label))
            listXdeShapeKind.push_back(to_QString(textIdTr("Component")));

        if (XCaf::isShapeCompound(label))
            listXdeShapeKind.push_back(to_QString(textIdTr("Compound")));

        if (XCaf::isShapeSimple(label))
            listXdeShapeKind.push_back(to_QString(textIdTr("Simple")));

        if (XCaf::isShapeSub(label))
            listXdeShapeKind.push_back(to_QString(textIdTr("Sub")));

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

    // Instance location
    if (XCaf::isShapeReference(label)) {
        const TopLoc_Location loc = XCaf::shapeReferenceLocation(label);
        m_propertyInstanceLocation.setValue(loc.Transformation());
    }
    else {
        this->removeProperty(&m_propertyInstanceLocation);
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

    // Product entity's properties
    if (XCaf::isShapeReference(label)) {
        m_labelProduct = XCaf::shapeReferred(label);

        m_propertyProductName.setValue(to_stdString(CafUtils::labelAttrStdName(m_labelProduct)));
        auto validProps = XCaf::validationProperties(m_labelProduct);
        m_propertyProductValidationCentroid.setValue(validProps.centroid);
        if (!validProps.hasCentroid)
            this->removeProperty(&m_propertyProductValidationCentroid);

        m_propertyProductValidationArea.setQuantity(validProps.area);
        if (!validProps.hasArea)
            this->removeProperty(&m_propertyProductValidationArea);

        m_propertyProductValidationVolume.setQuantity(validProps.volume);
        if (!validProps.hasVolume)
            this->removeProperty(&m_propertyProductValidationVolume);

        if (xcaf.hasShapeColor(m_labelProduct))
            m_propertyProductColor.setValue(xcaf.shapeColor(m_labelProduct));
        else
            this->removeProperty(&m_propertyProductColor);
    }
    else {
        this->removeProperty(&m_propertyProductName);
        this->removeProperty(&m_propertyProductValidationCentroid);
        this->removeProperty(&m_propertyProductValidationArea);
        this->removeProperty(&m_propertyProductValidationVolume);
        this->removeProperty(&m_propertyProductColor);
    }

    // User-defined attributes
    OccHandle<TDataStd_NamedData> data = xcaf.shapeUserDefinedAttributes(label);
    OccHandle<TDataStd_NamedData> productData = xcaf.shapeUserDefinedAttributes(m_labelProduct);
    m_textIdStringStorage.reserve(
        CafUtils::namedDataCount(data) + CafUtils::namedDataCount(productData)
    );
    addUdas(data, m_vecPropertyUda);
    addUdas(productData, m_vecPropertyProductUda);
    for (std::unique_ptr<Property>& propUda : m_vecPropertyUda)
        propUda->setUserData(XCafSubGroup_MetaData);

    for (std::unique_ptr<Property>& propUda : m_vecPropertyProductUda)
        propUda->setUserData(XCafSubGroup_ProductMetaData);

    for (Property* prop : this->properties())
        prop->setUserReadOnly(true);

    m_propertyName.setUserReadOnly(false);
    m_propertyProductName.setUserReadOnly(false);
}

void XCaf_DocumentTreeNodePropertiesProvider::Properties::onPropertyChanged(Property* prop)
{
    if (prop == &m_propertyName)
        TDataStd_Name::Set(m_label, to_OccExtString(m_propertyName.value()));
    else if (prop == &m_propertyProductName)
        TDataStd_Name::Set(m_labelProduct, to_OccExtString(m_propertyProductName.value()));

    PropertyGroup::onPropertyChanged(prop);
}

void XCaf_DocumentTreeNodePropertiesProvider::Properties::addUdas(
        const OccHandle<TDataStd_NamedData>& data,
        std::vector<std::unique_ptr<Property>>& vecProperty
    )
{
    std::vector<CafUtils::NamedDataKey> dataKeys = CafUtils::getNamedDataKeys(data);
    std::sort(
        dataKeys.begin(), dataKeys.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.label().IsLess(rhs.label()); }
    );
    for (const CafUtils::NamedDataKey& key : dataKeys) {
        assert(m_textIdStringStorage.size() < m_textIdStringStorage.capacity()); // Ensure no reallocation
        m_textIdStringStorage.push_back(to_stdString(key.label()));
        vecProperty.push_back(createProperty(key, textId(m_textIdStringStorage.back()), data));
    }
}

std::unique_ptr<Property> XCaf_DocumentTreeNodePropertiesProvider::Properties::createProperty(
        const CafUtils::NamedDataKey& key,
        const TextId& name,
        const OccHandle<TDataStd_NamedData>& data
    )
{
    // Helper function taking a TColStd_Array1Of[Integer/Real] and returns a string representation
    // Only the first 20 array items can appear in the string, otherwise it's elided with "..."
    auto fnToString = [](const auto& array) -> std::string {
        const int arrayTrimSize = std::min(20, array.Size());
        const bool isTrimArray = arrayTrimSize < array.Size();
        std::string strArray;
        for (auto it = array.cbegin(); it != (array.cbegin() + arrayTrimSize); ++it)
            strArray += std::to_string(*it) + "  ";

        if (arrayTrimSize < 10)
            return strArray;
        else if (!isTrimArray)
            return fmt::format("{} items: {}{}", array.Size(), strArray, isTrimArray ? "..." : "");

        return {};
    };

    switch (key.type) {
    case CafUtils::NamedDataType::None: {
        return {};
    }
    case CafUtils::NamedDataType::Int: {
        auto prop = std::make_unique<PropertyInt>(this, name);
        prop->setValue(data->GetInteger(key.label()));
        return prop;
    }
    case CafUtils::NamedDataType::Double: {
        auto prop = std::make_unique<PropertyDouble>(this, name);
        prop->setValue(data->GetReal(key.label()));
        return prop;
    }
    case CafUtils::NamedDataType::String: {
        auto prop = std::make_unique<PropertyString>(this, name);
        prop->setValue(to_stdString(data->GetString(key.label())));
        return prop;
    }
    case CafUtils::NamedDataType::Byte: {
        auto prop = std::make_unique<PropertyInt>(this, name, 0, 255, 1);
        prop->setValue(data->GetByte(key.label()));
        return prop;
    }
    case CafUtils::NamedDataType::IntArray: {
        auto prop = std::make_unique<PropertyString>(this, name);
        prop->setValue(fnToString(data->GetArrayOfIntegers(key.label())->Array1()));
        return prop;
    }
    case CafUtils::NamedDataType::DoubleArray: {
        auto prop = std::make_unique<PropertyString>(this, name);
        prop->setValue(fnToString(data->GetArrayOfReals(key.label())->Array1()));
        return prop;
    }
    } // endswitch()

    return {};
}



bool XCaf_DocumentTreeNodePropertiesProvider::supports(const DocumentTreeNode& treeNode) const
{
    return GraphicsShapeObjectDriver::shapeSupportStatus(treeNode.label()) == GraphicsObjectDriver::Support::Complete;
}

std::unique_ptr<PropertyGroup>
XCaf_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

TextId XCaf_DocumentTreeNodePropertiesProvider::subGroupLabelFromId(uint64_t id) const
{
    switch (id) {
    case XCafSubGroup_Core: return Properties::textId("Data");
    case XCafSubGroup_Validation: return Properties::textId("Validation");
    case XCafSubGroup_MetaData: return Properties::textId("MetaData");
    case XCafSubGroup_ProductMetaData: return Properties::textId("ProductMetaData");
    }
    return {};
}



class Mesh_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroup {
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
        for (Property* property : this->properties()) {
            property->setUserReadOnly(true);
            property->setUserData(GeneralCoreSubGroup);
        }
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

std::unique_ptr<PropertyGroup>
Mesh_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

TextId Mesh_DocumentTreeNodePropertiesProvider::subGroupLabelFromId(uint64_t id) const
{
    return id == GeneralCoreSubGroup ? Properties::textId("Data") : TextId{};
}



class PointCloud_DocumentTreeNodePropertiesProvider::Properties : public PropertyGroup {
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

        for (Property* property : this->properties()) {
            property->setUserReadOnly(true);
            property->setUserData(GeneralCoreSubGroup);
        }
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

std::unique_ptr<PropertyGroup>
PointCloud_DocumentTreeNodePropertiesProvider::properties(const DocumentTreeNode& treeNode) const
{
    if (!treeNode.isValid())
        return {};

    return std::make_unique<Properties>(treeNode);
}

TextId PointCloud_DocumentTreeNodePropertiesProvider::subGroupLabelFromId(uint64_t id) const
{
    return id == GeneralCoreSubGroup ? Properties::textId("Data") : TextId{};
}

} // namespace Mayo
