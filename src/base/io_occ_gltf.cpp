/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_gltf.h"
#include "property_builtins.h"

namespace Mayo {
namespace IO {

namespace {

struct GltfReaderParameters : public OccBaseMeshReaderParameters {
    GltfReaderParameters(PropertyGroup* parentGroup)
        : OccBaseMeshReaderParameters(parentGroup),
          skipEmptyNodes(this, MAYO_TEXT_ID("Mayo::IO::OccGltfReader", "skipEmptyNodes")),
          useMeshNameAsFallback(this, MAYO_TEXT_ID("Mayo::IO::OccGltfReader", "useMeshNameAsFallback"))
    {
    }

    PropertyBool skipEmptyNodes;
    PropertyBool useMeshNameAsFallback;
};

} // namespace

OccGltfReader::OccGltfReader()
    : OccBaseMeshReader(m_reader)
{
}

std::unique_ptr<PropertyGroup> OccGltfReader::createParameters(PropertyGroup* parentGroup)
{
    return std::make_unique<GltfReaderParameters>(parentGroup);
}

void OccGltfReader::applyParameters(const PropertyGroup* params)
{
    OccBaseMeshReader::applyParameters(params);
    auto ptr = dynamic_cast<const GltfReaderParameters*>(params);
    if (ptr) {
        this->setMeshNameAsFallback(ptr->useMeshNameAsFallback.value());
        this->setSkipEmptyNodes(ptr->skipEmptyNodes.value());
    }
}

} // namespace IO
} // namespace Mayo
