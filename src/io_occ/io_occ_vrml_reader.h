/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <VrmlAPI_CafReader.hxx>

namespace Mayo::IO {

// OpenCascade-based reader for VRML file format
// Requires OpenCascade >= v7.7.0
class OccVrmlReader : public OccBaseMeshReader {
public:
    OccVrmlReader();

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);

    OccBaseMeshReader::Parameters& parameters() override { return m_params; }
    const OccBaseMeshReader::Parameters& constParameters() const override { return m_params; }

private:
    VrmlAPI_CafReader m_reader;
    OccBaseMeshReader::Parameters m_params;
};

} // namespace Mayo::IO
