/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp_reader.h"
#include "../base/property.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <gp_Trsf.hxx>

namespace Mayo {
namespace IO {

class AssimpReader::Properties : public PropertyGroup {
    //MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::AssimpReader::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
    }
};

bool AssimpReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    // Assimp::Importer::SetProgressHandler()
    const unsigned flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
    m_scene = m_importer.ReadFile(filepath.u8string().c_str(), flags);
    if(!m_scene || (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !m_scene->mRootNode) {
        //qDebug() << "ERROR::ASSIMP:: " << t_importer.GetErrorString();
        return false;
    }

    return true;
}

static void transferNode(const aiNode* node, const aiScene* scene)
{
    // const QString nodeName = node->mName.C_Str();
    const aiMatrix4x4 matrix = node->mTransformation;
    const ai_real scalingX = aiVector3D(matrix.a1, matrix.a2, matrix.a3).Length();
    const ai_real scalingY = aiVector3D(matrix.b1, matrix.b2, matrix.b3).Length();
    const ai_real scalingZ = aiVector3D(matrix.c1, matrix.c2, matrix.c3).Length();

    gp_Trsf trsf;
    // TODO Check scalingXYZ != 0
    trsf.SetValues(matrix.a1 / scalingX, matrix.a2 / scalingX, matrix.a3 / scalingX, matrix.a4,
                   matrix.b1 / scalingY, matrix.b2 / scalingY, matrix.b3 / scalingY, matrix.b4,
                   matrix.c1 / scalingZ, matrix.c2 / scalingZ, matrix.c3 / scalingZ, matrix.c4);
}

bool AssimpReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (!m_scene)
        return false;

    transferNode(m_scene->mRootNode, m_scene);
    return true;
}

std::unique_ptr<PropertyGroup> AssimpReader::createProperties(PropertyGroup* parentGroup)
{
    return {};
}

void AssimpReader::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
    }
}

} // namespace IO
} // namespace Mayo

