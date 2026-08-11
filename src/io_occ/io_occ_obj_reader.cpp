/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_obj_reader.h"
#include "../base/property_builtins.h"

namespace Mayo::IO {

OccObjReader::Parameters::Parameters()
{
    this->restoreDefaults();
    this->singlePrecisionVertexCoords.setDescription(
        textId("Single precision flag for reading vertex data(coordinates)").tr()
    );
}

void OccObjReader::Parameters::restoreDefaults()
{
    OccBaseMeshReader::BaseParameters::restoreDefaults();
    this->singlePrecisionVertexCoords.setValue(false);
}

OccObjReader::OccObjReader()
    : OccBaseMeshReader(m_reader, m_params)
{
}

void OccObjReader::applyParameters()
{
    OccBaseMeshReader::applyParameters();
    m_reader.SetSinglePrecision(m_params.singlePrecisionVertexCoords);
}

} // namespace Mayo::IO
