namespace {
double dummy = 0;
}

#if 0

#include "src/io_occ/io_occ_common.h"
#include "src/io_occ/io_occ_stl.cpp"
#include "src/app/widget_model_tree_builder_xde.h"
namespace Mayo {

static void messages() {
    WidgetModelTreeBuilder_Xde::textId("Instance");
    WidgetModelTreeBuilder_Xde::textId("Product");
    WidgetModelTreeBuilder_Xde::textId("Both");
}

namespace IO {

static void messages() {
    OccCommon::textId("Undefined"); // RWMesh_CoordinateSystem_Undefined
    OccCommon::textId("posYfwd_posZup"); // RWMesh_CoordinateSystem_Zup
    OccCommon::textId("negZfwd_posYup"); // RWMesh_CoordinateSystem_Yup

    OccCommon::textId("Undefined");
    OccCommon::textId("Micrometer");
    OccCommon::textId("Millimeter");
    OccCommon::textId("Centimeter");
    OccCommon::textId("Meter");
    OccCommon::textId("Kilometer");
    OccCommon::textId("Inch");
    OccCommon::textId("Foot");
    OccCommon::textId("Mile");

    OccStlWriter::Properties::textId("Ascii");
    OccStlWriter::Properties::textId("Binary");
}

} // namespace IO
} // namespace Mayo

#endif
