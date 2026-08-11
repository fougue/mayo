/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <RWObj_CafReader.hxx>

namespace Mayo::IO {

// OpenCascade-based reader for Wavefront OBJ format
class OccObjReader : public OccBaseMeshReader {
public:
    OccObjReader();

    // Parameters
    struct Parameters : public OccBaseMeshReader::BaseParameters {
        PropertyBool singlePrecisionVertexCoords{ this, textId("singlePrecisionVertexCoords") };

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

protected:
    void applyParameters() override;

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccObjReader)

    Parameters m_params;
    RWObj_CafReader m_reader;
};

} // namespace Mayo::IO
