/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp_reader.h"
#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/math_utils.h"
#include "../base/mesh_utils.h"
#include "../base/messenger.h"
#include "../base/occ_handle.h"
#include "../base/property.h"
#include "../base/string_conv.h"
#include "../base/task_progress.h"
#include "../base/xcaf.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/postprocess.h>

#include <fmt/format.h>

#include <cassert>
#include <iostream>

#include <gp_Trsf.hxx>
#include <BRep_Builder.hxx>
#include <Image_Texture.hxx>
#include <Poly_Triangulation.hxx>
#include <TDataStd_Name.hxx>
#include <XCAFDoc_VisMaterial.hxx>
#include <XCAFDoc_VisMaterialCommon.hxx>
#include <XCAFDoc_VisMaterialPBR.hxx>

//#define MAYO_ASSIMP_READER_HANDLE_SCALING 1

namespace Mayo {
namespace IO {

namespace {

struct AssimpReaderI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::AssimpReaderI18N)
};

// Retrieve the scaling component in assimp matrix 'trsf'
aiVector3D aiMatrixScaling(const aiMatrix4x4& trsf)
{
    const ai_real scalingX = aiVector3D(trsf.a1, trsf.a2, trsf.a3).Length();
    const ai_real scalingY = aiVector3D(trsf.b1, trsf.b2, trsf.b3).Length();
    const ai_real scalingZ = aiVector3D(trsf.c1, trsf.c2, trsf.c3).Length();
    return aiVector3D(scalingX, scalingY, scalingZ);
}

bool hasScaleFactor(const gp_Trsf& trsf)
{
    const double topLocationScalePrec =
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        TopLoc_Location::ScalePrec();
#else
        1.e-14;
#endif
    return (Abs(Abs(trsf.ScaleFactor()) - 1.) > topLocationScalePrec) || trsf.IsNegative();
}

bool hasScaleFactor(const aiVector3D& scaling)
{
    return (!MathUtils::fuzzyEqual(scaling.x, 1)
            || !MathUtils::fuzzyEqual(scaling.y, 1)
            || !MathUtils::fuzzyEqual(scaling.z, 1)
            || scaling.x < 0. || scaling.y < 0. || scaling.z < 0.
           );
}

[[maybe_unused]] bool hasScaleFactor(const aiMatrix4x4& trsf)
{
    const aiVector3D scaling = aiMatrixScaling(trsf);
    return hasScaleFactor(scaling);
}

// Check if Assimp tree from 'node' contains a transformation having scale
bool deep_aiNodeTransformationHasScaling(const aiNode* node)
{
    const aiVector3D scaling = aiMatrixScaling(node->mTransformation);
    if (!MathUtils::fuzzyEqual(scaling.x, 1)
        || !MathUtils::fuzzyEqual(scaling.y, 1)
        || !MathUtils::fuzzyEqual(scaling.z, 1)
        || scaling.x < 0. || scaling.y < 0. || scaling.z < 0.
        )
    {
        //std::cout << "[TRACE] hasScaling: " << scaling.x << " " << scaling.y << " " <<scaling.z << std::endl;
        return true;
    }

    for (unsigned ichild = 0; ichild < node->mNumChildren; ++ichild) {
        if (deep_aiNodeTransformationHasScaling(node->mChildren[ichild]))
            return true;
    }

    return false;
}

// Visit each node in Assimp tree and call 'fnCallback'
void deep_aiNodeVisit(const aiNode* node, const std::function<void(const aiNode*)>& fnCallback)
{
    fnCallback(node);
    for (unsigned ichild = 0; ichild < node->mNumChildren; ++ichild)
        deep_aiNodeVisit(node->mChildren[ichild], fnCallback);
}

// Returns the OpenCascade transformation converted from assimp matrix
gp_Trsf toOccTrsf(const aiMatrix4x4& matrix)
{
    // TODO Check scaling != 0
    const aiVector3D scaling = aiMatrixScaling(matrix);
    gp_Trsf trsf;
    trsf.SetValues(
                matrix.a1 / scaling.x, matrix.a2 / scaling.x, matrix.a3 / scaling.x, matrix.a4,
                matrix.b1 / scaling.y, matrix.b2 / scaling.y, matrix.b3 / scaling.y, matrix.b4,
                matrix.c1 / scaling.z, matrix.c2 / scaling.z, matrix.c3 / scaling.z, matrix.c4
    );
    return trsf;
}

// Returns the Quantity_Color object equivalent to assimp 'color'
Quantity_Color toOccColor(const aiColor4D& color, Quantity_TypeOfColor colorType = Quantity_TOC_RGB)
{
    return Quantity_Color(color.r, color.g, color.b, colorType);
}

// Create an OpenCascade Image_Texture object from assimp texture
Handle(Image_Texture) createOccTexture(const aiTexture* texture)
{
    const auto textureWidth = texture->mWidth;
    const auto textureHeight = texture->mHeight;
    const auto textureSize = textureHeight == 0 ? textureWidth : 4 * textureWidth * textureHeight;
    Handle(NCollection_Buffer) buff = new NCollection_Buffer(
        NCollection_BaseAllocator::CommonBaseAllocator(),
        textureSize
    );
    auto textureData = reinterpret_cast<const Standard_Byte*>(texture->pcData);
    std::copy(textureData, textureData + textureSize, buff->ChangeData());
    return new Image_Texture(buff, texture->mFilename.C_Str());
}

// Create an OpenCascade Poly_Triangulation object from assimp mesh
// The input 'mesh' is assumed to contain only triangles
Handle(Poly_Triangulation) createOccTriangulation(const aiMesh* mesh)
{
    assert(mesh != nullptr);
    assert(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE);

    const unsigned textureIndex = 0;
    const bool hasUvNodes = mesh->HasTextureCoords(textureIndex) && mesh->mNumUVComponents[textureIndex] == 2;
    Handle(Poly_Triangulation) triangulation = new Poly_Triangulation(mesh->mNumVertices, mesh->mNumFaces, hasUvNodes);

    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& vertex = mesh->mVertices[i];
        MeshUtils::setNode(triangulation, i + 1, { vertex.x, vertex.y, vertex.z });
    }

    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        const auto indices = mesh->mFaces[i].mIndices;
        assert(mesh->mFaces[i].mNumIndices == 3);
        MeshUtils::setTriangle(triangulation, i + 1, Poly_Triangle(indices[0] + 1, indices[1] + 1, indices[2] + 1));
    }

    if (mesh->HasNormals()) {
        MeshUtils::allocateNormals(triangulation);
        for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
            using OccNormal = MeshUtils::Poly_Triangulation_NormalType;
            const aiVector3D& normal = mesh->mNormals[i];
            MeshUtils::setNormal(triangulation, i + 1, OccNormal{ normal.x, normal.y, normal.z });
        }
    }

    if (hasUvNodes) {
        for (unsigned i = 0; i < mesh->mNumVertices; i++) {
            const aiVector3D& t = mesh->mTextureCoords[textureIndex][i];
            MeshUtils::setUvNode(triangulation, i + 1, t.x, t.y);
        }
    }

    return triangulation;
}

// Provides assimp progress handler for TaskProgress
class AssimpProgressHandler : public Assimp::ProgressHandler {
public:
    AssimpProgressHandler(TaskProgress* progress)
        : m_progress(progress)
    {}

    bool Update(float percent = -1.f) override
    {
        if (TaskProgress::isAbortRequested(m_progress))
            return false;

        if (percent > 0)
            m_progress->setValue(percent * 100);

        return true;
    }

private:
    TaskProgress* m_progress = nullptr;
};

} // namespace

// --
// -- AssimpReader
// --

class AssimpReader::Properties : public PropertyGroup {
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
    }
};

bool AssimpReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    m_vecTriangulation.clear();
    m_vecMaterial.clear();
    m_mapMaterialLabel.clear();
    m_mapNodeData.clear();
    m_mapEmbeddedTexture.clear();
    m_mapFileTexture.clear();

    const unsigned flags =
            aiProcess_Triangulate
            | aiProcess_JoinIdenticalVertices
            //| aiProcess_SortByPType /* Crashes with assimp-5.3.1 on Windows */

            //| aiProcess_OptimizeGraph
            //| aiProcess_TransformUVCoords
            //| aiProcess_FlipUVs
            //| aiProcess_FindInstances
            //| aiProcess_PreTransformVertices
            //| aiProcess_FixInfacingNormals
            //| aiProcess_GlobalScale -> AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY
            //| aiProcess_GenNormals
            //| aiProcess_GenSmoothNormals
            //| aiProcess_GenUVCoords
            //| aiProcess_CalcTangentSpace
            //| aiProcess_ValidateDataStructure
            ;
    //const unsigned flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
    //m_importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    m_importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);
    m_importer.SetProgressHandler(new AssimpProgressHandler(progress));
    m_scene = m_importer.ReadFile(filepath.u8string(), flags);
    if (!m_scene || (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !m_scene->mRootNode) {
        this->messenger()->emitError(m_importer.GetErrorString());
        return false;
    }

#ifndef MAYO_ASSIMP_READER_HANDLE_SCALING
    // Apply "aiProcess_PreTransformVertices" post-processing step
    // This avoids issues with any non-identity scaling matrix
    // WARNING Assimp bones and animations are destructed by this step
    if (deep_aiNodeTransformationHasScaling(m_scene->mRootNode)) {
        m_scene = m_importer.ApplyPostProcessing(aiProcess_PreTransformVertices);
        this->messenger()->emitTrace("aiProcess_PreTransformVertices ON");
    }
#endif

    // Create OpenCascade elements from the assimp meshes
    //     mesh of triangles -> Poly_Triangulation
    //     mesh lines -> Poly_Polygon3D
    m_vecTriangulation.resize(m_scene->mNumMeshes);
    std::fill(m_vecTriangulation.begin(), m_vecTriangulation.end(), nullptr);
    for (unsigned i = 0; i < m_scene->mNumMeshes; ++i) {
        const aiMesh* mesh = m_scene->mMeshes[i];
        if (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
            auto triangulation = createOccTriangulation(mesh);
            m_vecTriangulation.at(i) = triangulation;
        }
        else if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE) {
            // TODO Create and add a Poly_Polygon3D object
            this->messenger()->emitWarning(AssimpReaderI18N::textIdTr("LINE primitives not supported yet"));
        }
        else {
            this->messenger()->emitWarning(AssimpReaderI18N::textIdTr("Some primitive not supported"));
        }
    }

    for (unsigned i = 0; i < m_scene->mNumTextures; ++i) {
        const aiTexture* texture = m_scene->mTextures[i];
        m_mapEmbeddedTexture.insert({ texture, createOccTexture(texture) });
    }

    m_vecMaterial.resize(m_scene->mNumMaterials);
    std::fill(m_vecMaterial.begin(), m_vecMaterial.end(), nullptr);
    for (unsigned i = 0; i < m_scene->mNumMaterials; ++i) {
        const aiMaterial* material = m_scene->mMaterials[i];
        m_vecMaterial.at(i) = this->createOccVisMaterial(material, filepath);
    }

    return true;
}

TDF_LabelSequence AssimpReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (!m_scene)
        return {};

    m_mapNodeData.clear();
    // Compute data for each aiNode object in the scene
    deep_aiNodeVisit(m_scene->mRootNode, [=](const aiNode* node) {
        aiNodeData nodeData;
        auto itParentData = m_mapNodeData.find(node->mParent);
        if (itParentData != m_mapNodeData.cend()) {
            const aiNodeData& parentData = itParentData->second;
            nodeData.aiAbsoluteTrsf = parentData.aiAbsoluteTrsf * node->mTransformation;
            nodeData.occAbsoluteTrsf = parentData.occAbsoluteTrsf * toOccTrsf(node->mTransformation);
        }
        else {
            nodeData.aiAbsoluteTrsf = node->mTransformation;
            nodeData.occAbsoluteTrsf = toOccTrsf(node->mTransformation);
        }

        m_mapNodeData.insert({ node, nodeData });
    });

    // Compute count of meshes in the scene
    int meshCount = 0;
    deep_aiNodeVisit(m_scene->mRootNode, [&](const aiNode* node) {
        for (unsigned imesh = 0; imesh < node->mNumMeshes; ++imesh)
            ++meshCount;
    });

    // Add materials in target document
    auto materialTool = doc->xcaf().visMaterialTool();
    for (const Handle_XCAFDoc_VisMaterial& material : m_vecMaterial) {
        const TDF_Label label = materialTool->AddMaterial(material, material->RawName()->String());
        m_mapMaterialLabel.insert({ material, label });
    }

    // Create entities in target document
    int imesh = 0;
    const TopoDS_Shape shapeEntity = BRepUtils::makeEmptyCompound();
    const TDF_Label labelEntity = doc->xcaf().shapeTool()->AddShape(shapeEntity, true/*makeAssembly*/);
    TDataStd_Name::Set(labelEntity, to_OccExtString(m_scene->mRootNode->mName.C_Str()));
    this->transferSceneNode(m_scene->mRootNode, doc, labelEntity, [&](const aiMesh*) {
        progress->setValue(MathUtils::toPercent(++imesh, 0, meshCount));
    });
    //doc->xcaf().shapeTool()->ComputeShapes(labelEntity);
    doc->xcaf().shapeTool()->UpdateAssemblies();
    return CafUtils::makeLabelSequence({ labelEntity });
}

std::unique_ptr<PropertyGroup> AssimpReader::createProperties(PropertyGroup* /*parentGroup*/)
{
    return {};
}

void AssimpReader::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
    }
}

Handle(Image_Texture) AssimpReader::findOccTexture(
    const std::string& strFilepath, const FilePath& modelFilepath
    )
{
    // Texture might be embedded
    {
        // Note: aiScene::GetEmbeddedTextureAndIndex() isn't available for version < 5.1
        const aiTexture* texture = m_scene->GetEmbeddedTexture(strFilepath.c_str());
        Handle(Image_Texture) occTexture = Cpp::findValue(texture, m_mapEmbeddedTexture);
        if (occTexture)
	    return occTexture;
    }

    // Texture might have already been loaded from file
    {
        Handle(Image_Texture) texture = CppUtils::findValue(strFilepath, m_mapFileTexture);
        if (texture)
            return texture;
    }

    // Fallback: load texture from filepath

    // Define texture "candidate" filepaths that will be tried
    const FilePath textureFilepath = filepathFrom(strFilepath);
    const FilePath textureFilepathCandidates[] = {
        textureFilepath,
        modelFilepath.parent_path() / textureFilepath,
        modelFilepath.parent_path() / textureFilepath.filename()
    };

    const FilePath* ptrTextureFilepath = nullptr;
    for (const FilePath& fp : textureFilepathCandidates) {
        if (filepathExists(fp)) {
            ptrTextureFilepath = &fp;
            break;
        }
    }

    // Could find an existing filepath for the texture
    if (ptrTextureFilepath) {
        Handle(Image_Texture) texture = new Image_Texture(filepathTo<TCollection_AsciiString>(*ptrTextureFilepath));
        // Cache texture
        m_mapFileTexture.insert({ strFilepath, texture });
        return texture;
    }

    // Report warning "texture not found"
    MessageStream msgWarning = this->messenger()->warning();
    msgWarning << fmt::format(AssimpReaderI18N::textIdTr("Texture not found: {}\nTried:"), strFilepath);
    for (const FilePath& fp : textureFilepathCandidates)
        msgWarning << "\n    " << filepathCanonical(fp).make_preferred().u8string();

    return {};
}

Handle(XCAFDoc_VisMaterial) AssimpReader::createOccVisMaterial(
    const aiMaterial* material, const FilePath& modelFilepath
    )
{
    Handle(XCAFDoc_VisMaterial) mat = new XCAFDoc_VisMaterial;

    //mat->SetAlphaMode(Graphic3d_AlphaMode_Opaque);

    ai_real shininessMax = 1.;
    std::string suffix = modelFilepath.extension().u8string();
    std::transform(suffix.cbegin(), suffix.cend(), suffix.begin(), [](char c) {
        return std::tolower(c, std::locale::classic());
    });
    if (suffix == ".fbx")
        shininessMax = 128.f;
    else if (suffix == ".obj")
        shininessMax = 1000.f;

    // Set name
    {
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        std::string_view vMatName{ matName.C_Str(), matName.length };
        mat->SetRawName(string_conv<Handle(TCollection_HAsciiString)>(vMatName));
    }

    // Backface culling
    {
        int flag;
        if (material->Get(AI_MATKEY_TWOSIDED, flag) == aiReturn_SUCCESS) {
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
            mat->SetFaceCulling(
                flag ?
                    Graphic3d_TypeOfBackfacingModel_DoubleSided
                     : Graphic3d_TypeOfBackfacingModel_BackCulled
            );
#else
            mat->SetDoubleSided(flag != 0);
#endif
        }
    }

    // Helper function to update some boolean flag if 'res' holds success status
    auto fnUpdateDefinedFlag = [](bool* ptrDefinedFlag, aiReturn res) {
        if (ptrDefinedFlag && res == aiReturn_SUCCESS)
            *ptrDefinedFlag = true;
        return res == aiReturn_SUCCESS;
    };

    // Helper function to get the color value of some property
    auto fnGetColor4D = [=](const char* key, unsigned type, unsigned index, aiColor4D* ptrColor, bool* ptrDefinedFlag = nullptr) {
        const aiReturn res = material->Get(key, type, index, *ptrColor);
        // Some models have wrong color components outside [0, 1] range, and Quantity_Color reject
        // them(exception in constructor)
        ptrColor->r = std::clamp(ptrColor->r, 0.f, 1.f);
        ptrColor->g = std::clamp(ptrColor->g, 0.f, 1.f);
        ptrColor->b = std::clamp(ptrColor->b, 0.f, 1.f);
        ptrColor->a = std::clamp(ptrColor->a, 0.f, 1.f);
        return fnUpdateDefinedFlag(ptrDefinedFlag, res);
    };

    // Helper function to get the value(real) of some property
    auto fnGetReal = [=](const char* key, unsigned type, unsigned index, ai_real* ptrValue, bool* ptrDefinedFlag = nullptr) {
        const aiReturn res = material->Get(key, type, index, *ptrValue);
        return fnUpdateDefinedFlag(ptrDefinedFlag, res);
    };

    // Helper function to get the texture path value of some property
    auto fnGetTexture = [=](aiTextureType type, aiString* ptrTexture, bool* ptrDefinedFlag = nullptr) {
        const aiReturn res = material->GetTexture(type, 0, ptrTexture);
        return fnUpdateDefinedFlag(ptrDefinedFlag, res);
    };

    // Helper function around AssimpReader::findOccTexture()
    auto fnFindOccTexture = [=](const aiString& strTexture) {
        return this->findOccTexture(strTexture.C_Str(), modelFilepath);
    };

    // Common
    XCAFDoc_VisMaterialCommon matCommon;
    matCommon.IsDefined = false;

    {
        aiColor4D color;
        if (fnGetColor4D(AI_MATKEY_COLOR_AMBIENT, &color, &matCommon.IsDefined))
            matCommon.AmbientColor = toOccColor(color);

        if (fnGetColor4D(AI_MATKEY_COLOR_DIFFUSE, &color, &matCommon.IsDefined))
            matCommon.DiffuseColor = toOccColor(color);

        if (fnGetColor4D(AI_MATKEY_COLOR_SPECULAR, &color, &matCommon.IsDefined))
            matCommon.SpecularColor = toOccColor(color);

#if 0
        ai_real factor;
        if (fnGetReal(AI_MATKEY_SPECULAR_FACTOR, &factor, &matCommon.IsDefined))
            this->messenger()->trace() << "AI_MATKEY_SPECULAR_FACTOR: " << factor;
#endif

        if (fnGetColor4D(AI_MATKEY_COLOR_EMISSIVE, &color, &matCommon.IsDefined))
            matCommon.EmissiveColor = toOccColor(color);
    }

    {
        ai_real value;
        if (fnGetReal(AI_MATKEY_OPACITY, &value, &matCommon.IsDefined))
            matCommon.Transparency = std::clamp(1.f - value, 0.f, 1.f);

        if (fnGetReal(AI_MATKEY_SHININESS, &value, &matCommon.IsDefined)) {
            matCommon.Shininess = std::clamp(value / shininessMax, 0.f, 1.f);
            //this->messenger()->trace() << "Shininess: " << value << " max: " << shininessMax;
        }
    }

    {
        aiString texDiffuse;
        if (fnGetTexture(aiTextureType_DIFFUSE, &texDiffuse, &matCommon.IsDefined))
            matCommon.DiffuseTexture = fnFindOccTexture(texDiffuse);
    }

    // PBR
    XCAFDoc_VisMaterialPBR matPbr;
    matPbr.IsDefined = false;

    {
        aiString strTexture;
        if (fnGetTexture(aiTextureType_BASE_COLOR, &strTexture, &matPbr.IsDefined))
            matPbr.BaseColorTexture = fnFindOccTexture(strTexture);

        if (fnGetTexture(aiTextureType_METALNESS, &strTexture))
            matPbr.MetallicRoughnessTexture = fnFindOccTexture(strTexture);

        if (fnGetTexture(aiTextureType_EMISSION_COLOR, &strTexture, &matPbr.IsDefined))
            matPbr.EmissiveTexture = fnFindOccTexture(strTexture);

        if (fnGetTexture(aiTextureType_AMBIENT_OCCLUSION, &strTexture, &matPbr.IsDefined))
            matPbr.OcclusionTexture = fnFindOccTexture(strTexture);

        if (fnGetTexture(aiTextureType_NORMALS, &strTexture, &matPbr.IsDefined))
            matPbr.NormalTexture = fnFindOccTexture(strTexture);
    }

#ifdef AI_MATKEY_BASE_COLOR
    {
        aiColor4D color;
        if (fnGetColor4D(AI_MATKEY_BASE_COLOR, &color, &matPbr.IsDefined))
            matPbr.BaseColor = Quantity_ColorRGBA(toOccColor(color), color.a);
    }
#endif

    {
        // TODO Handle EmissiveFactor
    }

    {
        ai_real value;
#ifdef AI_MATKEY_METALLIC_FACTOR
        if (fnGetReal(AI_MATKEY_METALLIC_FACTOR, &value, &matPbr.IsDefined))
            matPbr.Metallic = std::clamp(value, 0.f, 1.f);
#endif

#ifdef AI_MATKEY_ROUGHNESS_FACTOR
        if (fnGetReal(AI_MATKEY_ROUGHNESS_FACTOR, &value, &matPbr.IsDefined))
            matPbr.Roughness = std::clamp(value, 0.f, 1.f);
#endif

        if (fnGetReal(AI_MATKEY_REFRACTI, &value, &matPbr.IsDefined)) {
            // Refraction index must be in range [1.0, 3.0]
            // If < 1 then an exception is thrown by Graphic3d_MaterialAspect::SetRefractionIndex()
            matPbr.RefractionIndex = std::clamp(value, 1.f, 3.f);
        }
    }

    if (matCommon.IsDefined)
        mat->SetCommonMaterial(matCommon);

    if (matPbr.IsDefined)
        mat->SetPbrMaterial(matPbr);

    return mat;
}

void AssimpReader::transferSceneNode(
    const aiNode* node,
    DocumentPtr targetDoc,
    const TDF_Label& labelEntity,
    const std::function<void(const aiMesh*)>& fnCallbackMesh
    )
{
    if (!node)
        return;

    const std::string nodeName = node->mName.C_Str();

    const aiNodeData nodeData = Cpp::findValue(node, m_mapNodeData);
    gp_Trsf nodeAbsoluteTrsf = nodeData.occAbsoluteTrsf;
    if (hasScaleFactor(nodeAbsoluteTrsf))
        nodeAbsoluteTrsf.SetScaleFactor(1.);

#ifdef MAYO_ASSIMP_READER_HANDLE_SCALING
    const aiVector3D nodeScaling = aiMatrixScaling(nodeData.aiAbsoluteTrsf);
    const bool nodeHasScaling = hasScaleFactor(nodeScaling);
#endif

    // Produce shape corresponding to the node
    for (unsigned imesh = 0; imesh < node->mNumMeshes; ++imesh) {
        auto sceneMeshIndex = node->mMeshes[imesh];
        const aiMesh* mesh = m_scene->mMeshes[sceneMeshIndex];
        fnCallbackMesh(mesh);
        auto triangulation = m_vecTriangulation.at(sceneMeshIndex);
        if (!triangulation)
            continue; // Skip

#ifdef MAYO_ASSIMP_READER_HANDLE_SCALING
        if (nodeHasScaling) {
            triangulation = triangulation->Copy();
            for (int i = 1; i < triangulation->NbNodes(); ++i) {
                gp_Pnt pnt = triangulation->Node(i);
                pnt.SetX(pnt.X() * nodeScaling.x);
                pnt.SetY(pnt.Y() * nodeScaling.y);
                pnt.SetZ(pnt.Z() * nodeScaling.z);
                MeshUtils::setNode(triangulation, i , pnt);
            }
        }
#endif

        TopoDS_Face face = BRepUtils::makeFace(triangulation);
        face.Location(nodeAbsoluteTrsf);
        const TDF_Label labelComponent = targetDoc->xcaf().shapeTool()->AddComponent(labelEntity, face);
        const TDF_Label labelFace = targetDoc->xcaf().shapeReferred(labelComponent);

        if (mesh->mMaterialIndex < m_vecMaterial.size()) {
            const OccHandle<XCAFDoc_VisMaterial>& material = m_vecMaterial.at(mesh->mMaterialIndex);
            const TDF_Label materialLabel = Cpp::findValue(material, m_mapMaterialLabel);
            if (!materialLabel.IsNull())
                targetDoc->xcaf().visMaterialTool()->SetShapeMaterial(labelFace, materialLabel);
            else
                this->messenger()->trace() << "Material not found(umap), index: " << mesh->mMaterialIndex;
        }

        std::string shapeName = nodeName;
        if (node->mNumMeshes > 1) {
            shapeName += "_";
            if (mesh->mName.length > 0)
                shapeName += mesh->mName.C_Str();
            else
                shapeName += "mesh" + std::to_string(imesh);
        }

        TDataStd_Name::Set(labelFace, to_OccExtString(shapeName));
    }

    // Process child nodes
    for (unsigned ichild = 0; ichild < node->mNumChildren; ++ichild)
        this->transferSceneNode(node->mChildren[ichild], targetDoc, labelEntity, fnCallbackMesh);
}

} // namespace IO
} // namespace Mayo
