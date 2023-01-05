#include "label_data.h"

#include "caf_utils.h"
#include "brep_utils.h"
#include "triangulation_annex_data.h"
#include "point_cloud_data.h"
#include "xcaf.h"

namespace Mayo {

LabelDataFlags findLabelDataFlags(const TDF_Label& label)
{
    LabelDataFlags flags = LabelData_None;

    if (CafUtils::hasAttribute<TriangulationAnnexData>(label))
        flags |= LabelData_HasTriangulationAnnexData;

    if (XCaf::isShape(label)) {
        flags |= LabelData_HasShape;
        const TopoDS_Shape shape = XCaf::shape(label);
        if (shape.ShapeType() == TopAbs_FACE) {
            flags |= LabelData_ShapeIsFace;
            if (BRepUtils::isGeometric(TopoDS::Face(shape)))
                flags |= LabelData_ShapeIsGeometricFace;
        }
    }

    if (CafUtils::hasAttribute<PointCloudData>(label))
        flags |= LabelData_HasPointCloudData;

    return flags;
}

} // namespace Mayo
