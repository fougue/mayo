/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_gltf_reader.h"
#include "../base/property_builtins.h"

namespace Mayo::IO {

class OccGltfReader::Properties : public OccBaseMeshReaderProperties {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccGltfReader::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : OccBaseMeshReaderProperties(parentGroup)
    {
        this->skipEmptyNodes.setDescription(
            textIdTr("Ignore nodes without geometry(`Yes` by default)")
        );
        this->useMeshNameAsFallback.setDescription(
            textIdTr("Use mesh name in case if node name is empty(`Yes` by default)")
        );
    }

    void restoreDefaults() override
    {
        OccBaseMeshReaderProperties::restoreDefaults();
        this->skipEmptyNodes.setValue(true);
        this->useMeshNameAsFallback.setValue(true);
    }

    PropertyBool skipEmptyNodes{ this, textId("skipEmptyNodes") };
    PropertyBool useMeshNameAsFallback{ this, textId("useMeshNameAsFallback") };
};

OccGltfReader::OccGltfReader()
    : OccBaseMeshReader(m_reader)
{
}

std::unique_ptr<PropertyGroup> OccGltfReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccGltfReader::applyProperties(const PropertyGroup* params)
{
    OccBaseMeshReader::applyProperties(params);
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.useMeshNameAsFallback = ptr->useMeshNameAsFallback;
        m_params.skipEmptyNodes = ptr->skipEmptyNodes;
    }
}

void OccGltfReader::applyParameters()
{
    OccBaseMeshReader::applyParameters();
    m_reader.SetSkipEmptyNodes(m_params.skipEmptyNodes);
    m_reader.SetMeshNameAsFallback(m_params.useMeshNameAsFallback);
}

} // namespace Mayo::IO
