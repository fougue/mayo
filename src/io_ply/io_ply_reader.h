/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include <vector>

namespace Mayo {
namespace IO {

// Reader for PLY file format based on miniply library
class PlyReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    TDF_Label transferMesh(DocumentPtr doc, TaskProgress* progress);
    TDF_Label transferPointCloud(DocumentPtr doc, TaskProgress* progress);

    FilePath m_baseFilename;
    uint32_t m_nodeCount = 0;
    std::vector<float> m_vecNodeCoord;
    std::vector<int> m_vecIndex;
    std::vector<float> m_vecNormalCoord;
    std::vector<uint8_t> m_vecColorComponent;
};

// Provides factory to create PlyReader objects
class PlyFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Reader> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;
};

} // namespace IO
} // namespace Mayo
