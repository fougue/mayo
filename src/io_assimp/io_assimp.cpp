/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp.h"
#include "io_assimp_reader.h"

namespace Mayo {
namespace IO {

namespace {

const Format Format_3DS = { "3DS", "3DS Max File", { "3ds" } };
const Format Format_3MF = { "3MF", "3D Manufacturing Format", { "3mf" } };
const Format Format_COLLADA = { "COLLADA", "COLLAborative Design Activity(ISO/PAS 17506)", { "dae" } };
const Format Format_DXF = { "DXF", "Drawing eXchange Format", { "dxf" } };
const Format Format_FBX = { "FBX", "Filmbox", { "fbx" } };
const Format Format_IFC = { "IFC", "Industry Foundation Classes(ISO 16739)", { "ifc", "ifcXML", "ifczip" } };
const Format Format_OFF = { "OFF", "Object File Format", { "off" } };
const Format Format_PLY = { "PLY", "Polygon File Format", { "ply" } };
const Format Format_X3D = { "X3D", "Extensible 3D Graphics(ISO/IEC 19775/19776/19777)", { "x3d", "x3dv", "x3db", "x3dz", "x3dbz", "x3dvz" } };

} // namespace

Span<const Format> AssimpFactoryReader::formats() const
{
    static const Format array[] = {
        Format_AMF, Format_3DS, Format_3MF, Format_COLLADA, Format_DXF, Format_FBX, Format_GLTF,
        Format_IFC, Format_OBJ, Format_OFF, Format_PLY, Format_STL, Format_X3D
    };
    return array;
}

std::unique_ptr<Reader> AssimpFactoryReader::create(const Format& format) const
{
    auto itFound = std::find(this->formats().begin(), this->formats().end(), format);
    if (itFound != this->formats().end())
        return std::make_unique<AssimpReader>();

    return {};
}

std::unique_ptr<PropertyGroup>
AssimpFactoryReader::createProperties(const Format& format, PropertyGroup* parentGroup) const
{
    auto itFound = std::find(this->formats().begin(), this->formats().end(), format);
    if (itFound != this->formats().end())
        return AssimpReader::createProperties(parentGroup);

    return {};
}

} // namespace IO
} // namespace Mayo
