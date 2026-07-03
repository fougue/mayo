/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_single_format_factory.h"

#include <gp_Pnt.hxx>
#include <vector>
#include <type_traits>

namespace Mayo::IO {

// Reader for OFF file format
class OffReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup*) { return {}; }

private:
    TDF_Label transferMesh(DocumentPtr doc, TaskProgress* progress);
    TDF_Label transferPointCloud(DocumentPtr doc, TaskProgress* progress);

    struct Vertex {
        gp_Pnt coords;
        std::uint32_t color;
        bool hasColor = false;
    };

    struct Facet {
        int startIndexInArray;
        int vertexCount;
    };

    FilePath m_baseFilename;
    std::vector<Vertex> m_vecVertex;
    std::vector<int> m_vecAllFacetIndex;
    std::vector<Facet> m_vecFacet;
};

// Provides factory to create OffReader objects
class OffFactoryReader : public SingleFormatFactoryReader<Format_OFF, OffReader> {};

} // namespace Mayo::IO
