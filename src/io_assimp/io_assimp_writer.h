/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include "../base/occ_handle.h"

#include <Image_Texture.hxx>
#include <unordered_map>

class XCAFDoc_VisMaterial;

struct aiMaterial;
struct aiScene;
struct aiTexture;

namespace Mayo::IO {

// Assimp-based writer
// Requires OpenCascade >= v7.5.0(for XCAFDoc_VisMaterial)
class AssimpWriter : public Writer {
public:
    ~AssimpWriter();

    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& fp, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* group) override;

private:
    aiMaterial* createAssimpMaterial(const OccHandle<XCAFDoc_VisMaterial>& material) const;
    int indexOfEmbeddedTexture(const aiTexture*) const;
    std::string findAssimpTextureName(const OccHandle<Image_Texture>& tex) const;

    aiScene* m_scene = nullptr;
    std::unordered_map<OccHandle<Image_Texture>, aiTexture*> m_mapEmbeddedTexture;
};

} // namespace Mayo::IO
