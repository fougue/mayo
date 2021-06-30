/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp.h"
#include "io_assimp_reader.h"

namespace Mayo {
namespace IO {

Span<const Format> AssimpFactoryReader::formats() const
{
    static const Format array[] = {
        Format_AMF, Format_3DS, Format_3MF, Format_COLLADA, Format_DXF, Format_FBX, Format_GLTF,
        Format_IFC, Format_OBJ, Format_OFF, Format_PLY, Format_STL, Format_X3D
    };
    return array;
}

std::unique_ptr<Reader> AssimpFactoryReader::create(Format format) const
{
    auto itFound = std::find(this->formats().begin(), this->formats().end(), format);
    if (itFound != this->formats().end())
        return std::make_unique<AssimpReader>();

    return {};
}

std::unique_ptr<PropertyGroup>
AssimpFactoryReader::createProperties(Format format, PropertyGroup* parentGroup) const
{
    auto itFound = std::find(this->formats().begin(), this->formats().end(), format);
    if (itFound != this->formats().end())
        return AssimpReader::createProperties(parentGroup);

    return {};
}

} // namespace IO
} // namespace Mayo
