/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Mayo {
namespace IO {

struct Format {
    QByteArray identifier;
    QString name;
    QStringList fileSuffixes;
};

bool operator==(const Format& lhs, const Format& rhs);
bool operator!=(const Format& lhs, const Format& rhs);
unsigned hash(const Format& key);

// Predefined formats
const Format Format_Unknown = { "", "Format_Unknown", {} };
const Format Format_STEP = { "STEP", "STEP(ISO 10303)", { "stp", "step" } };
const Format Format_IGES = { "IGES", "IGES(ASME Y14.26M))", { "igs", "iges" } };
const Format Format_OCCBREP = { "OCCBREP", "OpenCascade BREP", { "brep", "rle", "occ" } };
const Format Format_STL = { "STL", "STL(STereo-Lithography)", { "stl" } };
const Format Format_OBJ = { "OBJ", "Wavefront OBJ", { "obj" } };
const Format Format_GLTF = { "GLTF", "glTF(GL Transmission Format)", { "gltf", "glb" } };
const Format Format_VRML = { "VRML", "VRML(ISO/CEI 14772-2)", { "wrl", "wrz", "vrml" } };
const Format Format_AMF = { "AMF", "Additive manufacturing file format(ISO/ASTM 52915:2016)", { "amf" } };

} // namespace IO
} // namespace Mayo
