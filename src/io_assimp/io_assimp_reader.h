/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"

#include <assimp/Importer.hpp>

#include <Image_Texture.hxx>
#include <Poly_Triangulation.hxx>
#include <XCAFDoc_VisMaterial.hxx>

#include <functional>
#include <unordered_map>
#include <vector>

struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiTexture;

namespace Mayo::IO {

// Assimp-based reader
// Requires OpenCascade >= v7.5.0(for XCAFDoc_VisMaterial)
class AssimpReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

private:
    // Create OpenCascade texture object
    // Parameter 'strFilepath' is the filepath to the texture as specified by the assimp material
    // Parameter 'modelFilepath' is the filepath to the 3D model being imported with Reader::readFile()
    OccHandle<Image_Texture> findOccTexture(const std::string& strFilepath, const FilePath& modelFilepath);

    // Create XCAFDoc_VisMaterial from assimp material
    // Parameter 'modelFilepath' is the filepath to the 3D model being imported with Reader::readFile()
    OccHandle<XCAFDoc_VisMaterial> createOccVisMaterial(const aiMaterial* material, const FilePath& modelFilepath);

    void transferSceneNode(
        const aiNode* node,
        DocumentPtr targetDoc,
        const TDF_Label& labelEntity,
        const std::function<void(const aiMesh*)>& fnCallbackMesh
    );

    struct aiNodeData {
        aiMatrix4x4 aiAbsoluteTrsf;
    };

    class Properties;
    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;

    std::vector<OccHandle<Poly_Triangulation>> m_vecTriangulation;
    std::vector<OccHandle<XCAFDoc_VisMaterial>> m_vecMaterial;
    std::unordered_map<OccHandle<XCAFDoc_VisMaterial>, TDF_Label> m_mapMaterialLabel;
    std::unordered_map<const aiNode*, aiNodeData> m_mapNodeData;
    std::unordered_map<const aiTexture*, OccHandle<Image_Texture>> m_mapEmbeddedTexture;
    std::unordered_map<std::string, OccHandle<Image_Texture>> m_mapFileTexture;
};

} // namespace Mayo::IO
