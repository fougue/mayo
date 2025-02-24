/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_obj_reader.h"
#include "../base/property_builtins.h"

namespace Mayo::IO {

class OccObjReader::Properties : public OccBaseMeshReaderProperties {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccObjReader::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : OccBaseMeshReaderProperties(parentGroup)
    {
        this->singlePrecisionVertexCoords.setDescription(
            textId("Single precision flag for reading vertex data(coordinates)").tr()
        );
    }

    void restoreDefaults() override
    {
        OccBaseMeshReaderProperties::restoreDefaults();
        this->singlePrecisionVertexCoords.setValue(false);
    }

    PropertyBool singlePrecisionVertexCoords{ this, textId("singlePrecisionVertexCoords") };
};

OccObjReader::OccObjReader()
    : OccBaseMeshReader(m_reader)
{
}

std::unique_ptr<PropertyGroup> OccObjReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccObjReader::applyProperties(const PropertyGroup* params)
{
    OccBaseMeshReader::applyProperties(params);
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.singlePrecisionVertexCoords = ptr->singlePrecisionVertexCoords;
    }
}

void OccObjReader::applyParameters()
{
    OccBaseMeshReader::applyParameters();
    m_reader.SetSinglePrecision(m_params.singlePrecisionVertexCoords);
}

} // namespace Mayo::IO
