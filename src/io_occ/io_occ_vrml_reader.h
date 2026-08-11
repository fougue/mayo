/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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

    using Parameters = OccBaseMeshReader::BaseParameters;

private:
    VrmlAPI_CafReader m_reader;
    BaseParameters m_params;
};

} // namespace Mayo::IO
