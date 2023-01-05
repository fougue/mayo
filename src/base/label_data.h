/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

class TDF_Label;

namespace Mayo {

enum LabelData {
    LabelData_None = 0x00000,
    LabelData_HasShape = 0x00001,    // Label has BRep shape attribute(see XCAFDoc_ShapeTool)
    LabelData_ShapeIsFace = 0x00002, // Associated BRep shape is a face(see TopoDS_Face)
    LabelData_ShapeIsGeometricFace = 0x00004, // BRep face is geometric(it's associated to a surface)
    LabelData_HasTriangulationAnnexData = 0x00080, // Label has TriangulationAnnexData attribute
    LabelData_HasPointCloudData = 0x00100, // Label has PointCloudData attribute
    LabelData_Custom1 = 0x01000,
    LabelData_Custom2 = 0x02000,
    LabelData_Custom3 = 0x03000,
    LabelData_Custom4 = 0x04000,
    LabelData_Custom5 = 0x08000
};
using LabelDataFlags = unsigned;

LabelDataFlags findLabelDataFlags(const TDF_Label& label);

} // namespace Mayo
