/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_gltf_reader.h"

namespace Mayo::IO {

OccGltfReader::Parameters::Parameters()
{
    this->restoreDefaults();
    this->skipEmptyNodes.setDescription(
        textId("Ignore nodes without geometry(`Yes` by default)")
    );
    this->useMeshNameAsFallback.setDescription(
        textId("Use mesh name in case if node name is empty(`Yes` by default)")
    );
}

void OccGltfReader::Parameters::restoreDefaults()
{
    OccBaseMeshReader::BaseParameters::restoreDefaults();
    this->skipEmptyNodes.setValue(true);
    this->useMeshNameAsFallback.setValue(true);
}

OccGltfReader::OccGltfReader()
    : OccBaseMeshReader(m_reader, m_params)
{
}

void OccGltfReader::applyParameters()
{
    OccBaseMeshReader::applyParameters();
    m_reader.SetSkipEmptyNodes(m_params.skipEmptyNodes);
    m_reader.SetMeshNameAsFallback(m_params.useMeshNameAsFallback);
}

} // namespace Mayo::IO
