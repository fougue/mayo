/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <RWObj_CafReader.hxx>

namespace Mayo::IO {

// OpenCascade-based reader for Wavefront OBJ format
// Requires OpenCascade >= v7.4.0
class OccObjReader : public OccBaseMeshReader {
public:
    OccObjReader();

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    struct Parameters : public OccBaseMeshReader::Parameters {
        bool singlePrecisionVertexCoords = false;
    };
    OccObjReader::Parameters& parameters() override { return m_params; }
    const OccObjReader::Parameters& constParameters() const override { return m_params; }

protected:
    void applyParameters() override;

private:
    class Properties;
    Parameters m_params;
    RWObj_CafReader m_reader;
};

} // namespace Mayo::IO
