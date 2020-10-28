/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_obj.h"
#include "property_builtins.h"

namespace Mayo {
namespace IO {

namespace {

struct ObjReaderParameters : public OccBaseMeshReaderParameters {
    ObjReaderParameters(PropertyGroup* parentGroup)
        : OccBaseMeshReaderParameters(parentGroup),
          singlePrecisionVertexCoords(this, MAYO_TEXT_ID("Mayo::IO::OccObjReader", "singlePrecisionVertexCoords"))
    {
    }

    PropertyBool singlePrecisionVertexCoords;
};

} // namespace

OccObjReader::OccObjReader()
    : OccBaseMeshReader(m_reader)
{
}

std::unique_ptr<PropertyGroup> OccObjReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<ObjReaderParameters>(parentGroup);
}

void OccObjReader::applyProperties(const PropertyGroup* params)
{
    OccBaseMeshReader::applyProperties(params);
    auto ptr = dynamic_cast<const ObjReaderParameters*>(params);
    if (ptr) {
        this->setSinglePrecisionVertexCoords(ptr->singlePrecisionVertexCoords.value());
    }
}

} // namespace IO
} // namespace Mayo
