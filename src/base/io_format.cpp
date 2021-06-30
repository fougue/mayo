/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_format.h"
#include "meta_enum.h"

#include <algorithm>

namespace Mayo {
namespace IO {

std::string_view formatIdentifier(Format format)
{
    if (format == Format_Unknown)
        return "";

    return MetaEnum::nameWithoutPrefix(format, "Format_");
}

std::string_view formatName(Format format)
{
    switch (format) {
    case Format_Unknown: return "Format_Unknown";
    case Format_AMF:  return "Additive manufacturing file format(ISO/ASTM 52915:2016)";
    case Format_GLTF: return "glTF(GL Transmission Format)";
    case Format_IGES: return "IGES(ASME Y14.26M)";
    case Format_OBJ:  return "Wavefront OBJ";
    case Format_OCCBREP: return "OpenCascade BREP";
    case Format_STEP: return "STEP(ISO 10303)";
    case Format_STL:  return "STL(STereo-Lithography)";
    case Format_VRML: return "VRML(ISO/CEI 14772-2)";
        //
    case Format_3DS: return "3DS Max File";
    case Format_3MF: return "3D Manufacturing Format";
    case Format_COLLADA: return "COLLAborative Design Activity(ISO/PAS 17506)";
    case Format_DXF: return "Drawing eXchange Format";
    case Format_FBX: return "Filmbox";
    case Format_IFC: return "Industry Foundation Classes(ISO 16739)";
    case Format_OFF: return "Object File Format";
    case Format_PLY: return "Polygon File Format";
    case Format_X3D: return "Extensible 3D Graphics(ISO/IEC 19775/19776/19777)";
    }

    return "";
}

Span<std::string_view> formatFileSuffixes(Format format)
{
    static std::string_view suffix_amf[] =  { "amf" };
    static std::string_view suffix_gltf[] = { "gltf", "glb" };
    static std::string_view suffix_iges[] = { "iges", "igs" };
    static std::string_view suffix_obj[] =  { "obj" };
    static std::string_view suffix_occ[] =  { "brep", "rle", "occ" };
    static std::string_view suffix_step[] = { "step", "stp" };
    static std::string_view suffix_stl[] =  { "stl" };
    static std::string_view suffix_vrml[] = { "wrl", "wrz", "vrml" };
    //
    static std::string_view suffix_3ds[] =  { "3ds" };
    static std::string_view suffix_3mf[] =  { "3mf" };
    static std::string_view suffix_collada[] =  { "dae" };
    static std::string_view suffix_dxf[] =  { "dxf" };
    static std::string_view suffix_fbx[] =  { "fbx" };
    static std::string_view suffix_ifc[] =  { "ifc", "ifcXML", "ifczip" };
    static std::string_view suffix_off[] =  { "off" };
    static std::string_view suffix_ply[] =  { "ply" };
    static std::string_view suffix_x3d[] =  { "x3d", "x3dv", "x3db", "x3dz", "x3dbz", "x3dvz" };

    switch (format) {
    case Format_Unknown: return {};
    case Format_AMF:  return suffix_amf;
    case Format_GLTF: return suffix_gltf;
    case Format_IGES: return suffix_iges;
    case Format_OBJ:  return suffix_obj;
    case Format_OCCBREP: return suffix_occ;
    case Format_STEP: return suffix_step;
    case Format_STL:  return suffix_stl;
    case Format_VRML: return suffix_vrml;
        //
    case Format_3DS: return suffix_3ds;
    case Format_3MF: return suffix_3mf;
    case Format_COLLADA: return suffix_collada;
    case Format_DXF: return suffix_dxf;
    case Format_FBX: return suffix_fbx;
    case Format_IFC: return suffix_ifc;
    case Format_OFF: return suffix_off;
    case Format_PLY: return suffix_ply;
    case Format_X3D: return suffix_x3d;
    }

    return {};
}

bool formatProvidesBRep(Format format)
{
    static const Format brepFormats[] = { Format_STEP, Format_IGES, Format_OCCBREP };
    return std::any_of(
                std::cbegin(brepFormats),
                std::cend(brepFormats),
                [=](Format candidate) { return candidate == format; });
}

bool formatProvidesMesh(Format format)
{
    return !formatProvidesBRep(format) && format != Format_Unknown;
}

} // namespace IO
} // namespace Mayo
