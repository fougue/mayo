/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
struct aiScene;

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
    aiScene* m_scene = nullptr;
};

} // namespace Mayo::IO
