/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp_writer.h"
#include "../base/caf_utils.h"
#include "../base/io_system.h"
#include "../base/mesh_access.h"
#include "../base/mesh_utils.h"
#include "../base/task_progress.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <Poly_Triangulation.hxx>

#include <unordered_set>
#include <vector>

namespace Mayo::IO {

namespace {

aiMesh* createAssimpMesh(const IMeshAccess& mesh)
{
    auto ai_mesh = new aiMesh;
    ai_mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

    // Vertices
    ai_mesh->mNumVertices = mesh.triangulation()->NbNodes();
    ai_mesh->mVertices = new aiVector3D[ai_mesh->mNumVertices];
    for (int i = 1; i <= mesh.triangulation()->NbNodes(); ++i) {
        auto node = mesh.triangulation()->Node(i);
        auto& ai_node = ai_mesh->mVertices[i - 1];
        ai_node.Set(ai_real(node.X()), ai_real(node.Y()), ai_real(node.Z()));
    }

    // Triangles
    ai_mesh->mNumFaces = mesh.triangulation()->NbTriangles();
    ai_mesh->mFaces = new aiFace[ai_mesh->mNumFaces];
    const Poly_Array1OfTriangle& triangles = MeshUtils::triangles(mesh.triangulation());
    for (int i = 1; i <= triangles.Size(); ++i) {
        const auto& tri = triangles.Value(i);
        auto& ai_face = ai_mesh->mFaces[i - 1];
        ai_face.mNumIndices = 3;
        ai_face.mIndices = new unsigned int[3];
        ai_face.mIndices[0] = tri.Value(1);
        ai_face.mIndices[1] = tri.Value(2);
        ai_face.mIndices[2] = tri.Value(3);
    }

    // Normals
    if (mesh.triangulation()->HasNormals()) {
        ai_mesh->mNormals = new aiVector3D[ai_mesh->mNumVertices];
        for (int i = 1; i < mesh.triangulation()->NbNodes(); ++i) {
            auto n = MeshUtils::normal(mesh.triangulation(), i);
            auto& ai_n = ai_mesh->mNormals[i - 1];
            n.Normalize();
            ai_n.Set(MeshUtils::normalX(n), MeshUtils::normalY(n), MeshUtils::normalZ(n));
        }
    }

    // Texture coords
    if (mesh.triangulation()->HasUVNodes()) {
        ai_mesh->mTextureCoords[0] = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mNumUVComponents[0] = 2;
        for (int i = 1; i < mesh.triangulation()->NbNodes(); ++i) {
            auto uv = mesh.triangulation()->UVNode(i);
            auto& ai_tc = ai_mesh->mTextureCoords[0][i - 1];
            ai_tc.Set(ai_real(uv.X()), ai_real(uv.Y()), 0.);
        }
    }

    return ai_mesh;
}

} // namespace

AssimpWriter::~AssimpWriter()
{
    delete m_scene;
}

bool AssimpWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();

    // Create materials
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& treeNode) {

    });

    // Find meshes
    std::unordered_set<TDF_Label> setOfPartLabels;
    std::vector<aiMesh*> vecMesh;
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& treeNode) {
        TDF_Label partLabel;
        if (treeNode.isLeaf()) {
            if (XCaf::isShapeReference(treeNode.label()))
                partLabel = XCaf::shapeReferred(treeNode.label());
            else
                partLabel = treeNode.label();
        }

        if (!partLabel.IsNull() && setOfPartLabels.find(partLabel) == setOfPartLabels.cend()) {
            setOfPartLabels.insert(partLabel);
            // TODO Handle sub shapes with XCaf::shapeSubs(partLabel)
            IMeshAccess_visitMeshes(treeNode, [&](const IMeshAccess& mesh) {
                auto ai_mesh = createAssimpMesh(mesh);
                vecMesh.push_back(ai_mesh);
            });
        }
    });


    delete m_scene;
    m_scene = new aiScene;
    m_scene->mRootNode = new aiNode("Root Node");

    return false;
}

bool AssimpWriter::writeFile(const FilePath& fp, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();
    return false;
}

std::unique_ptr<PropertyGroup> AssimpWriter::createProperties(PropertyGroup* parentGroup)
{
    return {};
}

void AssimpWriter::applyProperties(const PropertyGroup* group)
{

}

} // namespace Mayo::IO
