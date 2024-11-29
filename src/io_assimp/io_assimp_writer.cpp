/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp_writer.h"

#include "io_assimp_i18n.h"
#include "io_assimp_task_progress.h"

#include "../base/caf_utils.h"
#include "../base/filepath_conv.h"
#include "../base/io_system.h"
#include "../base/mesh_access.h"
#include "../base/mesh_utils.h"
#include "../base/messenger.h"
#include "../base/meta_enum.h"
#include "../base/string_conv.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <assimp/Exporter.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <Poly_Triangulation.hxx>
#include <XCAFDoc_VisMaterial.hxx>

#include <fmt/format.h>
#include <algorithm>
#include <fstream>
#include <locale>
#include <unordered_set>
#include <vector>

namespace Mayo::IO {

namespace {

using ai_MaterialPtr = aiMaterial*;
using ai_MeshPtr = aiMesh*;
using ai_TexturePtr = aiTexture*;

bool isTextureFilePortion(const OccHandle<Image_Texture>& tex)
{
    return !tex->FilePath().IsEmpty() && tex->FileOffset() != -1;
}

bool isTextureDataBuffer(const OccHandle<Image_Texture>& tex)
{
    return tex->DataBuffer() && !tex->DataBuffer()->IsEmpty();
}

aiColor4D toAssimpColor(const Quantity_Color& color)
{
    aiColor4D ai_color = {};
    ai_color.r = static_cast<ai_real>(color.Red());
    ai_color.g = static_cast<ai_real>(color.Green());
    ai_color.b = static_cast<ai_real>(color.Blue());
    return ai_color;
}

aiColor4D toAssimpColor(const Quantity_ColorRGBA& color)
{
    aiColor4D ai_color = toAssimpColor(color.GetRGB());
    ai_color.a = color.Alpha();
    return ai_color;
}

ai_MeshPtr createAssimpMesh(const IMeshAccess& mesh)
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

// Helper function to return `str` converted to lower case characters
// The input string `str` is assumed to be encoded with the classic "C" locale
std::string stringToLowerCase_c(std::string_view str)
{
    auto charToLowerCase_c = [](char c) {
        return std::tolower(c, std::locale::classic());
    };
    std::string strLc;
    std::transform(str.cbegin(), str.cend(), strLc.begin(), charToLowerCase_c);
    return strLc;
}

// Helper function to return suffix string(utf8) from filepath `fp`
// Note: any starting '.' character is removed from the result string
std::string filepathSuffix(const FilePath& fp)
{
    std::string suffix = stringToLowerCase_c(fp.extension().u8string());
    if (!suffix.empty() && suffix.front() == '.')
        suffix.erase(suffix.begin());

    return suffix;
}

// Helper function to retrieve the Format corresponding to `suffix`
// Assumes that `suffix` is lowercase and does not start with '.'
Format findFormatFromFileSuffix(std::string_view suffix)
{
    for (auto format : MetaEnum::values<Format>()) {
        auto formatSuffixes = formatFileSuffixes(format);
        auto itSuffix = std::find(formatSuffixes.begin(), formatSuffixes.end(), suffix);
        if (itSuffix != formatSuffixes.end())
            return format;
    }

    return Format_Unknown;
}

// Helper function to find the identifier of the Assimp export format corresponding to `format`
std::string findAssimpExportFormatId(const Assimp::Exporter& exporter, Format format)
{
    const size_t formatCount = exporter.GetExportFormatCount();
    for (size_t i = 0; i < formatCount; ++i) {
        const aiExportFormatDesc* exportFormat = exporter.GetExportFormatDescription(i);
        if (exportFormat == nullptr)
            continue; //Skip

        if (exportFormat->id && findFormatFromFileSuffix(exportFormat->id) == format)
            return exportFormat->id;

        if (exportFormat->fileExtension) {
            const std::string exportFormatSuffix = stringToLowerCase_c(exportFormat->fileExtension);
            if (findFormatFromFileSuffix(exportFormatSuffix) == format)
                return exportFormat->id;
        }
    }

    return {};
}

} // namespace

AssimpWriter::~AssimpWriter()
{
    delete m_scene;
}

bool AssimpWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();

    // Find materials
    std::unordered_set<OccHandle<XCAFDoc_VisMaterial>> setVisMaterial;
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& treeNode) {
        auto visMatTool = treeNode.document()->xcaf().visMaterialTool();
        auto visMat = visMatTool->GetShapeMaterial(treeNode.label());
        if (visMat)
            setVisMaterial.insert(visMat);
    });

    // Find meshes
    std::unordered_set<TDF_Label> setOfPartLabels;
    std::vector<ai_MeshPtr> vecMesh;
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

    m_mapEmbeddedTexture.clear();

    delete m_scene;
    m_scene = new aiScene;

    m_scene->mNumMeshes = static_cast<unsigned>(vecMesh.size());
    m_scene->mMeshes = new ai_MeshPtr[vecMesh.size()];
    for (unsigned i = 0; i < vecMesh.size(); ++i)
        m_scene->mMeshes[i] = vecMesh[i];

    // Create Assimp embedded textures
    std::vector<OccHandle<Image_Texture>> vecTextureToEmbed;
    auto fnMaybeEmbedTexture = [&](const OccHandle<Image_Texture>& tex) {
        if (isTextureFilePortion(tex) || isTextureDataBuffer(tex))
            vecTextureToEmbed.push_back(tex);
    };
    for (const OccHandle<XCAFDoc_VisMaterial>& visMat : setVisMaterial) {
        const XCAFDoc_VisMaterialPBR& pbr = visMat->PbrMaterial();
        fnMaybeEmbedTexture(pbr.BaseColorTexture);
        fnMaybeEmbedTexture(pbr.MetallicRoughnessTexture);
        fnMaybeEmbedTexture(pbr.EmissiveTexture);
        fnMaybeEmbedTexture(pbr.OcclusionTexture);
        fnMaybeEmbedTexture(pbr.NormalTexture);
    }

    m_scene->mNumTextures = unsigned(vecTextureToEmbed.size());
    m_scene->mTextures = new ai_TexturePtr[vecTextureToEmbed.size()];
    for (unsigned i = 0; i < vecTextureToEmbed.size(); ++i) {
        auto aiTex = new aiTexture;
        const auto& occTex = vecTextureToEmbed.at(i);
        if (isTextureFilePortion(occTex)) {
            std::ifstream file(filepathFrom(occTex->FilePath()));
            if (file.is_open()) {
                file.seekg(occTex->FileOffset());
                auto texData = new char[occTex->FileLength()];
                file.read(texData, occTex->FileLength());
                aiTex->mWidth = unsigned(file.gcount());
                aiTex->pcData = reinterpret_cast<aiTexel*>(texData);
            }
            else {
                // TODO Report error when texture file failed to open
            }
        }
        else if (isTextureDataBuffer(occTex)) {
            aiTex->mWidth = unsigned(occTex->DataBuffer()->Size());
            aiTex->pcData = reinterpret_cast<aiTexel*>(occTex->DataBuffer()->ChangeData());
        }

        aiTex->mFilename.Set("*" + std::to_string(i));
        m_scene->mTextures[i] = aiTex;
        m_mapEmbeddedTexture.insert({ occTex, aiTex });
    }

    // Create Assimp materials
    m_scene->mNumMaterials = unsigned(setVisMaterial.size());
    m_scene->mMaterials = new ai_MaterialPtr[setVisMaterial.size()];
    unsigned iMaterial = 0;
    for (const OccHandle<XCAFDoc_VisMaterial>& visMat : setVisMaterial) {
        m_scene->mMaterials[iMaterial] = this->createAssimpMaterial(visMat);
        ++iMaterial;
    }

    // Create Assimp node graph
    m_scene->mRootNode = new aiNode("Root Node");

    return false;
}

bool AssimpWriter::writeFile(const FilePath& fp, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();
    Assimp::Exporter exporter;
    exporter.SetProgressHandler(new AssimpTaskProgress(progress));

    const std::string fileSuffix = filepathSuffix(fp);
    const Format format = findFormatFromFileSuffix(fileSuffix);
    const std::string aiFormatId = findAssimpExportFormatId(exporter, format);
    if (aiFormatId.empty()) {
        this->messenger()->emitError(
            fmt::format(AssimpI18N::textIdTr("No Assimp export format corresponding to file suffix '{}'"), fileSuffix)
        );
        return false;
    }

    const aiReturn errExport = exporter.Export(m_scene, aiFormatId, fp.u8string());
    if (errExport != aiReturn_SUCCESS) {
        this->messenger()->emitError(exporter.GetErrorString());
        return false;
    }

    return true;
}

std::unique_ptr<PropertyGroup> AssimpWriter::createProperties(PropertyGroup* parentGroup)
{
    return {};
}

void AssimpWriter::applyProperties(const PropertyGroup* /*group*/)
{
}

ai_MaterialPtr AssimpWriter::createAssimpMaterial(const OccHandle<XCAFDoc_VisMaterial>& material) const
{
    auto ai_material = new aiMaterial;

    // Helper function to add OpenCascade texture 'tex' as a property into assimp material 'mat'
    auto fnAddTextureProperty = [=](aiMaterial* mat, const OccHandle<Image_Texture>& tex, aiTextureType texType) {
        const aiString texName{ this->findAssimpTextureName(tex) };
        if (texName.length != 0)
            mat->AddProperty(&texName, AI_MATKEY_TEXTURE(texType, 0));
        // TODO Report error if `texName`is empty
    };

    // Name
    {
        const aiString name{ to_stdString(material->RawName()) };
        ai_material->AddProperty(&name, AI_MATKEY_NAME);
    }

    // Backface culling
    {
        int flag = 0;
        //if (material->Get(AI_MATKEY_TWOSIDED, flag) == aiReturn_SUCCESS) {
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        flag = material->FaceCulling() == Graphic3d_TypeOfBackfacingModel_DoubleSided;
#else
        flag = material->IsDoubleSided() ? 1 : 0;
#endif
        ai_material->AddProperty(&flag, 1, AI_MATKEY_TWOSIDED);
    }

    // Common
    if (material->CommonMaterial().IsDefined) {
        const XCAFDoc_VisMaterialCommon& common = material->CommonMaterial();
        {
            aiColor4D color = toAssimpColor(common.AmbientColor);
            ai_material->AddProperty(&color, 1, AI_MATKEY_COLOR_AMBIENT);

            color = toAssimpColor(common.DiffuseColor);
            ai_material->AddProperty(&color, 1, AI_MATKEY_COLOR_DIFFUSE);

            color = toAssimpColor(common.SpecularColor);
            ai_material->AddProperty(&color, 1, AI_MATKEY_COLOR_DIFFUSE);
        }

        {
            ai_real value = 1 - common.Transparency;
            ai_material->AddProperty(&value, 1, AI_MATKEY_OPACITY);

            constexpr ai_real shininessMax = 1.;
            value = shininessMax * common.Shininess;
            ai_material->AddProperty(&value, 1, AI_MATKEY_SHININESS);
        }

        fnAddTextureProperty(ai_material, common.DiffuseTexture, aiTextureType_DIFFUSE);
    }

    // PBR
    if (material->PbrMaterial().IsDefined) {
        const XCAFDoc_VisMaterialPBR& pbr = material->PbrMaterial();

        aiString texName;
        ai_material->AddProperty(&texName, AI_MATKEY_BASE_COLOR_TEXTURE);

        fnAddTextureProperty(ai_material, pbr.BaseColorTexture, aiTextureType_BASE_COLOR);
        fnAddTextureProperty(ai_material, pbr.MetallicRoughnessTexture, aiTextureType_METALNESS);
        fnAddTextureProperty(ai_material, pbr.EmissiveTexture, aiTextureType_EMISSION_COLOR);
        fnAddTextureProperty(ai_material, pbr.OcclusionTexture, aiTextureType_AMBIENT_OCCLUSION);
        fnAddTextureProperty(ai_material, pbr.NormalTexture, aiTextureType_NORMALS);

        aiColor4D color = {};
        ai_real value = 0.;

#ifdef AI_MATKEY_BASE_COLOR
        {
            color = toAssimpColor(pbr.BaseColor);
            ai_material->AddProperty(&color, 1, AI_MATKEY_BASE_COLOR);
        }
#endif

#ifdef AI_MATKEY_METALLIC_FACTOR
        {
            value = pbr.Metallic;
            ai_material->AddProperty(&value, 1, AI_MATKEY_METALLIC_FACTOR);
        }
#endif

#ifdef AI_MATKEY_ROUGHNESS_FACTOR
        {
            value = pbr.Roughness;
            ai_material->AddProperty(&value, 1, AI_MATKEY_ROUGHNESS_FACTOR);
        }
#endif

        {
            value = pbr.RefractionIndex;
            ai_material->AddProperty(&value, 1, AI_MATKEY_REFRACTI);
        }
    }

    return ai_material;
}

int AssimpWriter::indexOfEmbeddedTexture(const aiTexture* tex) const
{
    if (!tex || !m_scene)
        return -1;

    auto texBegin = m_scene->mTextures;
    auto texEnd = m_scene->mTextures + m_scene->mNumTextures;
    auto it = std::find(m_scene->mTextures, m_scene->mTextures + m_scene->mNumTextures, tex);
    return it != texEnd ? int(texEnd - texBegin) : -1;
}

std::string AssimpWriter::findAssimpTextureName(const OccHandle<Image_Texture>& tex) const
{
    std::string texName;
    if (isTextureFilePortion(tex) || isTextureDataBuffer(tex)) {
        auto itAssimpTex = m_mapEmbeddedTexture.find(tex);
        const aiTexture* aiTex = itAssimpTex != m_mapEmbeddedTexture.cend() ? itAssimpTex->second : nullptr;
        const int idTex = this->indexOfEmbeddedTexture(aiTex);
        if (idTex != -1)
            texName = "*" + std::to_string(idTex);
    }
    else if (!tex->FilePath().IsEmpty()) {
        texName = to_stdString(tex->FilePath());
    }

    return texName;
}

} // namespace Mayo::IO
