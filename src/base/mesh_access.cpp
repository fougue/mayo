/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mesh_access.h"

#include "brep_utils.h"
#include "caf_utils.h"
#include "cpp_utils.h"
#include "data_triangulation.h"
#include "document.h"
#include "document_tree_node.h"

#include <BRep_Tool.hxx>
#include <Standard_Version.hxx>
#include <TopoDS_Face.hxx>
#if OCC_VERSION_HEX >= 0x070500
#  include <XCAFDoc_VisMaterial.hxx>
#endif

#include <optional>

namespace Mayo {

// Provides mesh access to a TopoDS_Face object stored within XCAF document
class XCafFace_MeshAccess : public IMeshAccess {
public:
    XCafFace_MeshAccess(const DocumentTreeNode& treeNode, const TopoDS_Face& face)
    {
        const DocumentPtr& doc = treeNode.document();
        const TDF_Label labelNode = treeNode.label();
        if (doc->xcaf().isShapeSubOf(labelNode, face))
            m_faceColor = findShapeColor(doc, doc->xcaf().findShapeLabel(face));

        if (!m_faceColor)
            m_faceColor = findShapeColor(doc, labelNode);

        const TopLoc_Location locShape = XCaf::shapeAbsoluteLocation(doc->modelTree(), treeNode.id());
        TopLoc_Location locFace;
        m_triangulation = BRep_Tool::Triangulation(face, locFace);
        m_location = locShape * locFace;
    }

    std::optional<Quantity_Color> nodeColor(int /*i*/) const override {
        return m_faceColor;
    }

    const TopLoc_Location& location() const override {
        return m_location;
    }

    const Handle(Poly_Triangulation)& triangulation() const override {
        return m_triangulation;
    }

private:
    static std::optional<Quantity_Color> findShapeColor(const DocumentPtr& doc, const TDF_Label& labelShape)
    {
        if (doc->xcaf().hasShapeColor(labelShape))
            return doc->xcaf().shapeColor(labelShape);

#if OCC_VERSION_HEX >= 0x070500
        Handle_XCAFDoc_VisMaterialTool visMaterialTool = doc->xcaf().visMaterialTool();
        Handle_XCAFDoc_VisMaterial visMaterial = visMaterialTool->GetShapeMaterial(labelShape);
        if (visMaterial)
            return visMaterial->BaseColor().GetRGB();
#endif

        return {};
    }

    std::optional<Quantity_Color> m_faceColor;
    TopLoc_Location m_location;
    Handle(Poly_Triangulation) m_triangulation;
};

// Provides access to a mesh stored within a DataTriangulation attribute
class DataTriangulation_MeshAccess : public IMeshAccess {
public:
    DataTriangulation_MeshAccess(const DocumentTreeNode& treeNode)
        : m_dataTriangulation(CafUtils::findAttribute<DataTriangulation>(treeNode.label()))
    {
    }

    std::optional<Quantity_Color> nodeColor(int i) const override
    {
        if (0 <= i && CppUtils::cmpLess(i, m_dataTriangulation->nodeColors().size()))
            return m_dataTriangulation->nodeColors()[i];
        else
            return {};
    }

    const TopLoc_Location& location() const override {
        return m_location;
    }

    const Handle(Poly_Triangulation)& triangulation() const override {
        return m_dataTriangulation->Get();
    }

private:
    opencascade::handle<DataTriangulation> m_dataTriangulation;
    TopLoc_Location m_location;
};

void IMeshAccess_visitMeshes(
        const DocumentTreeNode& treeNode,
        std::function<void(const IMeshAccess&)> fnCallback)
{
    if (!fnCallback || !treeNode.isValid())
        return;

    auto fnProxyCallback = [&](const IMeshAccess& mesh) {
        if (mesh.triangulation())
            fnCallback(mesh);
    };
    if (XCaf::isShape(treeNode.label())) {
        BRepUtils::forEachSubFace(XCaf::shape(treeNode.label()), [&](const TopoDS_Face& face) {
            fnProxyCallback(XCafFace_MeshAccess(treeNode, face));
        });
    }
    else if (CafUtils::hasAttribute<DataTriangulation>(treeNode.label())) {
        fnProxyCallback(DataTriangulation_MeshAccess(treeNode));
    }
}

} // namespace Mayo
