/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_format.h"

#include <algorithm>

namespace Mayo {
namespace IO {

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
    }

    return "";
}

Span<std::string_view> formatFileSuffixes(Format format)
{
    static std::string_view img_suffix[]  = { "bmp", "jpeg", "jpg", "png", "gif", "ppm", "tiff" };
    static std::string_view step_suffix[] = { "step", "stp" };
    static std::string_view iges_suffix[] = { "iges", "igs" };
    static std::string_view occ_suffix[]  = { "brep", "rle", "occ" };
    static std::string_view stl_suffix[]  = { "stl" };
    static std::string_view obj_suffix[]  = { "obj" };
    static std::string_view gltf_suffix[] = { "gltf", "glb" };
    static std::string_view vrml_suffix[] = { "wrl", "wrz", "vrml" };
    static std::string_view amf_suffix[]  = { "amf" };
    static std::string_view dxf_suffix[]  = { "dxf" };
    static std::string_view ply_suffix[]  = { "ply" };
    static std::string_view off_suffix[]  = { "off" };

    switch (format) {
    case Format_Unknown: return {};
    case Format_Image: return img_suffix;
    case Format_STEP:  return step_suffix;
    case Format_IGES:  return iges_suffix;
    case Format_OCCBREP: return occ_suffix;
    case Format_STL:   return stl_suffix;
    case Format_OBJ:   return obj_suffix;
    case Format_GLTF:  return gltf_suffix;
    case Format_VRML:  return vrml_suffix;
    case Format_AMF:   return amf_suffix;
    case Format_DXF:   return dxf_suffix;
    case Format_PLY:   return ply_suffix;
    case Format_OFF:   return off_suffix;
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
            && format != Format_Image;
}

} // namespace IO
} // namespace Mayo
