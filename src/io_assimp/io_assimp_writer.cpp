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

AssimpWriter::~AssimpWriter()
{
    delete m_scene;
}

bool AssimpWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();

    // Find unique application items
    std::vector<ApplicationItem> uniqueAppItems;
    System::visitUniqueItems(appItems, [&](const ApplicationItem& item) {
        uniqueAppItems.push_back(item);
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
            IMeshAccess_visitMeshes(treeNode, [&](const IMeshAccess& mesh) {
                auto assimpMesh = new aiMesh;
                assimpMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
                assimpMesh->mNumVertices = mesh.triangulation()->NbNodes();
                assimpMesh->mVertices = new aiVector3D[assimpMesh->mNumVertices];
                for (int i = 1; i <= mesh.triangulation()->NbNodes(); ++i) {
                    const gp_Pnt node = mesh.triangulation()->Node(i);
                    assimpMesh->mVertices[i - 1].Set(
                        ai_real(node.X()), ai_real(node.Y()), ai_real(node.Z())
                    );
                }

                assimpMesh->mNumFaces = mesh.triangulation()->NbTriangles();
                assimpMesh->mFaces = new aiFace[assimpMesh->mNumFaces];
                const Poly_Array1OfTriangle& triangles = MeshUtils::triangles(mesh.triangulation());
                for (int i = 1; i <= triangles.Size(); ++i) {
                    const Poly_Triangle& tri = triangles.Value(i);
                    auto& assimpFace = assimpMesh->mFaces[i - 1];
                    assimpFace.mNumIndices = 3;
                    assimpFace.mIndices = new unsigned int[3];
                    assimpFace.mIndices[0] = tri.Value(1);
                    assimpFace.mIndices[1] = tri.Value(2);
                    assimpFace.mIndices[2] = tri.Value(3);
                }

                if (mesh.triangulation()->HasNormals()) {
                    assimpMesh->mNormals = new aiVector3D[assimpMesh->mNumVertices];
                    for (int i = 1; i < mesh.triangulation()->NbNodes(); ++i) {
                        auto n = MeshUtils::normal(mesh.triangulation(), i);
                        n.Normalize();
                        assimpMesh->mNormals[i - 1].Set(
                            MeshUtils::normalX(n), MeshUtils::normalY(n), MeshUtils::normalZ(n)
                        );
                    }
                }

                if (mesh.triangulation()->HasUVNodes()) {
                    //assimpMesh->mTextureCoords = new aiVector3D[assimpMesh->mNumVertices];
                    for (int i = 1; i < mesh.triangulation()->NbNodes(); ++i) {
                    }
                }

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
