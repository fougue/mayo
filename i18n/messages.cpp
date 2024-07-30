namespace {
double dummy = 0;
}

#include "src/io_gmio/io_gmio_amf_writer.cpp"
#include "src/io_image/io_image.cpp"
#include "src/io_occ/io_occ_common.h"
#include "src/io_occ/io_occ_stl.cpp"
#include "src/io_occ/io_occ_gltf_writer.cpp"
#include "src/io_ply/io_ply_writer.cpp"
#include "src/app/app_module.h"
#include "src/app/widget_model_tree_builder_xde.h"

static void messages() {
    // App
    Mayo::AppModuleProperties::textId("SI");
    Mayo::AppModuleProperties::textId("ImperialUK");

    Mayo::AppModuleProperties::textId("VeryCoarse");
    Mayo::AppModuleProperties::textId("Coarse");
    Mayo::AppModuleProperties::textId("Normal");
    Mayo::AppModuleProperties::textId("Precise");
    Mayo::AppModuleProperties::textId("VeryPrecise");
    Mayo::AppModuleProperties::textId("UserDefined");

    Mayo::AppModuleProperties::textId("None");
    Mayo::AppModuleProperties::textId("ReloadIfUserConfirm");
    Mayo::AppModuleProperties::textId("ReloadSilently");

    Mayo::WidgetModelTreeBuilder_Xde::textId("Instance");
    Mayo::WidgetModelTreeBuilder_Xde::textId("Product");
    Mayo::WidgetModelTreeBuilder_Xde::textId("Both");

    // I/O
    Mayo::IO::GmioAmfWriter::Properties::textId("Decimal");
    Mayo::IO::GmioAmfWriter::Properties::textId("Scientific");
    Mayo::IO::GmioAmfWriter::Properties::textId("Shortest");

    Mayo::IO::OccCommon::textId("Undefined"); // RWMesh_CoordinateSystem_Undefined
    Mayo::IO::OccCommon::textId("posYfwd_posZup"); // RWMesh_CoordinateSystem_Zup
    Mayo::IO::OccCommon::textId("negZfwd_posYup"); // RWMesh_CoordinateSystem_Yup

    Mayo::IO::OccCommon::textId("Undefined");
    Mayo::IO::OccCommon::textId("Micrometer");
    Mayo::IO::OccCommon::textId("Millimeter");
    Mayo::IO::OccCommon::textId("Centimeter");
    Mayo::IO::OccCommon::textId("Meter");
    Mayo::IO::OccCommon::textId("Kilometer");
    Mayo::IO::OccCommon::textId("Inch");
    Mayo::IO::OccCommon::textId("Foot");
    Mayo::IO::OccCommon::textId("Mile");

    Mayo::IO::OccStlWriterI18N::textId("Ascii");
    Mayo::IO::OccStlWriterI18N::textId("Binary");

    Mayo::IO::OccGltfWriter::Properties::textId("Json");
    Mayo::IO::OccGltfWriter::Properties::textId("Binary");

    Mayo::IO::PlyWriterI18N::textId("Ascii");
    Mayo::IO::PlyWriterI18N::textId("Binary");

    Mayo::IO::ImageWriterI18N::textId("Perspective");
    Mayo::IO::ImageWriterI18N::textId("Orthographic");
}
