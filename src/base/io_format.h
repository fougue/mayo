/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <gsl/span>
#include <string_view>

namespace Mayo::IO {

// Predefined I/O formats
enum Format {
    Format_Unknown,
    Format_Image,
    Format_3DS,
    Format_3MF,
    Format_AMF,
    Format_COLLADA,
    Format_DXF,
    Format_FBX,
    Format_GLTF,
    Format_IGES,
    Format_OBJ,
    Format_OCCBREP,
    Format_OFF,
    Format_PLY,
    Format_STEP,
    Format_STL,
    Format_VRML,
    Format_X3D,
    Format_DirectX,
    Format_Blender
};

// Returns identifier(unique short name) corresponding to 'format'
std::string_view formatIdentifier(Format format);

// Returns name(eg ISO designation) corresponding to 'format'
std::string_view formatName(Format format);

// Returns array of applicable file suffixes(extensions) corresponding to 'format'
gsl::span<std::string_view> formatFileSuffixes(Format format);

// Does 'format' provide BRep model ?
bool formatProvidesBRep(Format format);

// Does 'format' provide mesh model ?
bool formatProvidesMesh(Format format);

} // namespace Mayo::IO
