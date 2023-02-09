/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_vrml_reader.h"

namespace Mayo {
namespace IO {

OccVrmlReader::OccVrmlReader()
    : OccBaseMeshReader(m_reader)
{
}

std::unique_ptr<PropertyGroup> OccVrmlReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<OccBaseMeshReaderProperties>(parentGroup);
}

} // namespace IO
} // namespace Mayo
