/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <string_view>

namespace Mayo {
namespace IO {

// Predefined I/O formats
enum Format {
    Format_Unknown,
    Format_Image,
    Format_STEP,
    Format_IGES,
    Format_OCCBREP,
    Format_STL,
    Format_OBJ,
    Format_GLTF,
    Format_VRML,
    Format_AMF,
    Format_DXF,
    Format_PLY
};

// Returns identifier(unique short name) corresponding to 'format'
std::string_view formatIdentifier(Format format);

// Returns name(eg ISO designation) corresponding to 'format'
std::string_view formatName(Format format);

// Returns array of applicable file suffixes(extensions) corresponding to 'format'
Span<std::string_view> formatFileSuffixes(Format format);

// Does 'format' provide BRep model ?
bool formatProvidesBRep(Format format);

// Does 'format' provide mesh model ?
bool formatProvidesMesh(Format format);

} // namespace IO
} // namespace Mayo
