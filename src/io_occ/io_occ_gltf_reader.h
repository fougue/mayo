/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <RWGltf_CafReader.hxx>

namespace Mayo::IO {

// OpenCascade-based reader for glTF format
class OccGltfReader : public OccBaseMeshReader {
public:
    OccGltfReader();

    // Parameters
    struct Parameters : public OccBaseMeshReader::BaseParameters {
        PropertyBool skipEmptyNodes{ this, textId("skipEmptyNodes") };
        PropertyBool useMeshNameAsFallback{ this, textId("useMeshNameAsFallback") };

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

protected:
    void applyParameters() override;

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccGltfReader)

    Parameters m_params;
    RWGltf_CafReader m_reader;
};

} // namespace Mayo::IO
