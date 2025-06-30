/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "libtree.h"
#include "occ_handle.h"
#include "quantity.h"

#include <Quantity_Color.hxx>
#include <Standard_Version.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_Material.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#if OCC_VERSION_HEX >= 0x070500
#  include <XCAFDoc_VisMaterialTool.hxx>
#endif
#if OCC_VERSION_HEX < 0x070400
#  include <TDataStd_NamedData.hxx>
#endif

namespace Mayo {

// Closely related to Mayo::Document
class XCaf {
public:
    struct ValidationProperties {
        bool hasCentroid;
        bool hasArea;
        bool hasVolume;
        gp_Pnt centroid;
        QuantityArea area;
        QuantityVolume volume;
    };

    bool isNull() const;

    OccHandle<XCAFDoc_ShapeTool> shapeTool() const;
    OccHandle<XCAFDoc_LayerTool> layerTool() const;
    OccHandle<XCAFDoc_ColorTool> colorTool() const;
    OccHandle<XCAFDoc_MaterialTool> materialTool() const;
#if OCC_VERSION_HEX >= 0x070500
    OccHandle<XCAFDoc_VisMaterialTool> visMaterialTool() const;
#endif

    // --
    // -- XCAFDoc_ShapeTool  helpers
    // --

    TDF_LabelSequence topLevelFreeShapes() const;
    static TDF_LabelSequence shapeComponents(const TDF_Label& lbl);
    static TDF_LabelSequence shapeSubs(const TDF_Label& lbl);

    static TopoDS_Shape shape(const TDF_Label& lbl);
    void setShape(const TDF_Label& label, const TopoDS_Shape& shape);

    static bool isShape(const TDF_Label& lbl);
    static bool isShapeFree(const TDF_Label& lbl);
    static bool isShapeAssembly(const TDF_Label& lbl);
    static bool isShapeReference(const TDF_Label& lbl);
    static bool isShapeSimple(const TDF_Label& lbl);
    static bool isShapeComponent(const TDF_Label& lbl);
    static bool isShapeCompound(const TDF_Label& lbl);
    static bool isShapeSub(const TDF_Label& lbl);

    // Is 'shape' a subshape of the shape stored in 'lbl' ?
    bool isShapeSubOf(const TDF_Label& lbl, const TopoDS_Shape& shape);

    // Does shape stored at 'lbl' have at least one user?
    // Returns 'false' is shape is free
    static bool hasShapeUsers(const TDF_Label& lbl);

    // Various flags for findShapeLabel()
    enum FindShapeLabelFlag {
        // findShapeLabel() will first try to find the shape among the top-level shapes
        FindShapeLabel_Instance = 0x01,
        // findShapeLabel() will try to find the shape find the shape among the components of assemblies
        FindShapeLabel_Component = 0x02,
        // findShapeLabel() will try tries to find a shape as a subshape of top-level simple shapes
        FindShapeLabel_SubShape = 0x04,
        // Enables all flags
        FindShapeLabel_All = 0xFF
    };
    using FindShapeLabelFlags = unsigned;

    // Finds a (sub) shape in the document, returns null label if not found
    TDF_Label findShapeLabel(const TopoDS_Shape& shape, FindShapeLabelFlags flags = FindShapeLabel_All) const;

    TopLoc_Location shapeAbsoluteLocation(TreeNodeId nodeId) const;
    static TopLoc_Location shapeAbsoluteLocation(const Tree<TDF_Label>& modelTree, TreeNodeId nodeId);
    static TopLoc_Location shapeReferenceLocation(const TDF_Label& lbl);
    static TDF_Label shapeReferred(const TDF_Label& lbl);

    // Returns labels of the top-level free shapes that were not found in 'seqOther'
    TDF_LabelSequence diffTopLevelFreeShapes(const TDF_LabelSequence& seqOther) const;

    OccHandle<TDataStd_NamedData> shapeUserDefinedAttributes(const TDF_Label& lbl) const;

    // --
    // -- XCAFDoc_ColorTool helpers
    // --

    bool hasShapeColor(const TDF_Label& lbl) const;
    Quantity_Color shapeColor(const TDF_Label& lbl) const;

    // --
    // -- XCAFDoc_Material helpers
    // --

    static QuantityDensity shapeMaterialDensity(const TDF_Label& lbl);
    static QuantityDensity shapeMaterialDensity(const OccHandle<XCAFDoc_Material>& material);
    static OccHandle<XCAFDoc_Material> shapeMaterial(const TDF_Label& lbl);

    // --
    // -- XCAFDoc_LayerTool helpers
    // --

    TDF_LabelSequence layers(const TDF_Label& lbl) const;
    TCollection_ExtendedString layerName(const TDF_Label& lbl) const;

    static ValidationProperties validationProperties(const TDF_Label& lbl);

private:
    XCaf() = default;

    TreeNodeId deepBuildAssemblyTree(TreeNodeId parentNode, const TDF_Label& label);
    void setLabelMain(const TDF_Label& labelMain) { m_labelMain = labelMain; }
    void setModelTree(Tree<TDF_Label>& modelTree) { m_modelTree = &modelTree; }

    friend class Document;
    TDF_Label m_labelMain;
    Tree<TDF_Label>* m_modelTree = nullptr;
};

} // namespace Mayo
