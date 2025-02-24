/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
