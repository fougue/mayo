/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <RWGltf_CafReader.hxx>

namespace Mayo::IO {

// OpenCascade-based reader for glTF format
// Requires OpenCascade >= v7.4.0
class OccGltfReader : public OccBaseMeshReader {
public:
    OccGltfReader();

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    struct Parameters : public OccBaseMeshReader::Parameters {
        bool skipEmptyNodes = true;
        bool useMeshNameAsFallback = true;
    };
    OccGltfReader::Parameters& parameters() override { return m_params; }
    const OccGltfReader::Parameters& constParameters() const override { return m_params; }

protected:
    void applyParameters() override;

private:
    class Properties;
    Parameters m_params;
    RWGltf_CafReader m_reader;
};

} // namespace Mayo::IO
