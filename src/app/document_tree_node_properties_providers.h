/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_tree_node_properties_provider.h"
#include "../base/property_builtins.h"

#include <QtCore/QCoreApplication>
#include <TDF_Label.hxx>

namespace Mayo {

class XCaf_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
    Q_DECLARE_TR_FUNCTIONS(XCaf_DocumentTreeNodePropertiesProvider)
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const override;

private:
    struct InternalPropertyGroup : public PropertyGroupSignals {
        InternalPropertyGroup(const DocumentTreeNode& treeNode);

        void onPropertyChanged(Property* prop) override;

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
};

class Mesh_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
    Q_DECLARE_TR_FUNCTIONS(Mesh_DocumentTreeNodePropertiesProvider)
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const override;

private:
    struct InternalPropertyGroup : public PropertyGroupSignals {
        InternalPropertyGroup(const DocumentTreeNode& treeNode);
        PropertyInt m_propertyNodeCount; // Read-only
        PropertyInt m_propertyTriangleCount; // Read-only
    };
};

} // namespace Mayo
