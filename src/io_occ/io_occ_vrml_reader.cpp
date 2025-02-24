/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_vrml_reader.h"

namespace Mayo::IO {

OccVrmlReader::OccVrmlReader()
    : OccBaseMeshReader(m_reader)
{
    // Fixes weird "mirroring" of the loaded model
    m_reader.SetFileLengthUnit(1.);
#if 0
    double scaleFactor = 1.;
    if (!XCAFDoc_DocumentTool::GetLengthUnit(doc, scaleFactor))
        scaleFactor = UnitsMethods::GetCasCadeLengthUnit();
    m_reader.SetSystemLengthUnit(scaleFactor);
#endif
}

std::unique_ptr<PropertyGroup> OccVrmlReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<OccBaseMeshReaderProperties>(parentGroup);
}

} // namespace Mayo::IO
