/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_format.h"

#include <algorithm>

namespace Mayo::IO {

std::string_view formatIdentifier(Format format)
{
    switch (format) {
    case Format_Unknown: return "";
    case Format_Image: return "Image";
    case Format_STEP:  return "STEP";
    case Format_IGES:  return "IGES";
    case Format_OCCBREP: return "OCCBREP";
    case Format_STL:   return "STL";
    case Format_OBJ:   return "OBJ";
    case Format_GLTF:  return "GLTF";
    case Format_VRML:  return "VRML";
    case Format_AMF:   return "AMF";
    case Format_DXF:   return "DXF";
    case Format_PLY:   return "PLY";
    case Format_OFF:   return "OFF";
    case Format_3DS:   return "3DS";
    case Format_3MF:   return "3MF";
    case Format_COLLADA: return "COLLADA";
    case Format_FBX:   return "FBX";
    case Format_X3D:   return "X3D";
    case Format_DirectX: return "X";
    case Format_Blender: return "Blender";
    }

    return "";
}

std::string_view formatName(Format format)
{
    switch (format) {
    case Format_Unknown: return "Format_Unknown";
    case Format_Image: return "Image";
    case Format_STEP:  return "STEP(ISO 10303)";
    case Format_IGES:  return "IGES(ASME Y14.26M)";
    case Format_OCCBREP: return "OpenCascade BREP";
    case Format_STL:   return "STL(STereo-Lithography)";
    case Format_OBJ:   return "Wavefront OBJ";
    case Format_GLTF:  return "glTF(GL Transmission Format)";
    case Format_VRML:  return "VRML(ISO/CEI 14772-2)";
    case Format_AMF:   return "Additive manufacturing file format(ISO/ASTM 52915:2016)";
    case Format_DXF:   return "Drawing Exchange Format";
    case Format_PLY:   return "Polygon File Format";
    case Format_OFF:   return "Object File Format";
    case Format_3DS:   return "3DS Max File";
    case Format_3MF:   return "33D Manufacturing Format";
    case Format_COLLADA: return "COLLAborative Design Activity(ISO/PAS 17506)";
    case Format_FBX:   return "Filmbox";
    case Format_X3D:   return "Extensible 3D Graphics(ISO/IEC 19775/19776/19777)";
    case Format_DirectX: return "DirectX File Format";
    case Format_Blender: return "Blender File Format";
    }

    return "";
}

Span<std::string_view> formatFileSuffixes(Format format)
{
    static std::string_view suffix_img[]  = { "bmp", "jpeg", "jpg", "png", "gif", "ppm", "tiff" };
    static std::string_view suffix_3ds[]  = { "3ds" };
    static std::string_view suffix_3mf[]  = { "3mf" };
    static std::string_view suffix_amf[]  = { "amf" };
    static std::string_view suffix_collada[] = { "dae", "zae" };
    static std::string_view suffix_dxf[]  = { "dxf" };
    static std::string_view suffix_fbx[]  = { "fbx" };
    static std::string_view suffix_gltf[] = { "gltf", "glb" };
    static std::string_view suffix_iges[] = { "iges", "igs" };
    static std::string_view suffix_obj[]  = { "obj" };
    static std::string_view suffix_occ[]  = { "brep", "rle", "occ" };
    static std::string_view suffix_off[]  = { "off" };
    static std::string_view suffix_ply[]  = { "ply" };
    static std::string_view suffix_step[] = { "step", "stp" };
    static std::string_view suffix_stl[]  = { "stl" };
    static std::string_view suffix_vrml[] = { "wrl", "wrz", "vrml" };
    static std::string_view suffix_x3d[]  = { "x3d", "x3dv", "x3db", "x3dz", "x3dbz", "x3dvz" };
    static std::string_view suffix_directx[]  = { "x" };
    static std::string_view suffix_blender[]  = { "blend", "blender", "blend1", "blend2" };

    switch (format) {
    case Format_Unknown: return {};
    case Format_Image: return suffix_img;
    case Format_3DS:   return suffix_3ds;
    case Format_3MF:   return suffix_3mf;
    case Format_AMF:   return suffix_amf;
    case Format_COLLADA: return suffix_collada;
    case Format_DXF:   return suffix_dxf;
    case Format_FBX:   return suffix_fbx;
    case Format_GLTF:  return suffix_gltf;
    case Format_IGES:  return suffix_iges;
    case Format_OBJ:   return suffix_obj;
    case Format_OCCBREP: return suffix_occ;
    case Format_OFF:   return suffix_off;
    case Format_PLY:   return suffix_ply;
    case Format_STEP:  return suffix_step;
    case Format_STL:   return suffix_stl;
    case Format_VRML:  return suffix_vrml;
    case Format_X3D:   return suffix_x3d;
    case Format_DirectX: return suffix_directx;
    case Format_Blender: return suffix_blender;
    }

    return {};
}

bool formatProvidesBRep(Format format)
{
    static const Format brepFormats[] = { Format_STEP, Format_IGES, Format_OCCBREP, Format_DXF };
    return std::any_of(
                std::cbegin(brepFormats),
                std::cend(brepFormats),
                [=](Format candidate) { return candidate == format; }
    );
}

bool formatProvidesMesh(Format format)
{
    return !formatProvidesBRep(format)
            && format != Format_Unknown
            && format != Format_Image
        ;
}

} // namespace Mayo::IO
