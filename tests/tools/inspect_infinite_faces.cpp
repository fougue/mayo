/****************************************************************************
** Copyright (c) 2026, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

// A diagnostic executable, deliberately using the production Mayo shape driver.
// Usage: inspect-infinite-faces input.step output-dir [shaded_edges|shaded|wireframe|all]
// Run the same executable source against unmodified and patched MayoCoreLib to
// compare presentation bounds and screenshots without changing the imported data.

#include "../../src/base/application.h"
#include "../../src/base/document.h"
#include "../../src/graphics/graphics_scene.h"
#include "../../src/graphics/graphics_shape_object_driver.h"
#include "../../src/graphics/graphics_utils.h"
#include "../../src/io_image/io_image.h"

#include <AIS_ColoredDrawer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <Geom_Surface.hxx>
#include <Graphic3d_Camera.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Poly_Triangulation.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Version.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <XCAFPrs_AISObject.hxx>

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using Mayo::GraphicsShapeObjectDriver;

struct Root {
    TDF_Label label;
    TopoDS_Shape shape;
    Mayo::GraphicsObjectPtr graphics;
    std::string beforeBrep;
};

struct Mode {
    const char* name;
    GraphicsShapeObjectDriver::DisplayMode value;
};

QJsonValue number(double value)
{
    if (std::isfinite(value))
        return value;

    return std::isnan(value) ? "NaN" : (value < 0 ? "-Infinity" : "+Infinity");
}

QJsonArray coordinates(double x, double y, double z)
{
    return {number(x), number(y), number(z)};
}

QJsonObject boxRecord(const Bnd_Box& box)
{
    QJsonObject record{{"void", box.IsVoid()}, {"open", box.IsOpen()}};
    if (box.IsVoid())
        return record;

    double xMin, yMin, zMin, xMax, yMax, zMax;
    box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    record["min"] = coordinates(xMin, yMin, zMin);
    record["max"] = coordinates(xMax, yMax, zMax);
    record["dimensions"] = coordinates(xMax - xMin, yMax - yMin, zMax - zMin);
    record["finite_dimensions"] = std::isfinite(xMax - xMin)
        && std::isfinite(yMax - yMin) && std::isfinite(zMax - zMin) && !box.IsOpen();
    return record;
}

std::string serialize(const TopoDS_Shape& shape)
{
    // Include triangulations: compare display-only changes after the one explicit
    // meshing step below, not the expected addition of meshing caches on import.
    std::ostringstream stream;
    BRepTools::Write(shape, stream);
    return stream.str();
}

QString sha256(const std::string& bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(
        QByteArray(bytes.data(), static_cast<int>(bytes.size())), QCryptographicHash::Sha256
    ).toHex());
}

void writeBytes(const QString& filename, const QByteArray& bytes)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size())
        throw std::runtime_error("Cannot write " + filename.toStdString());
}

QJsonObject topologyRecord(const Root& root)
{
    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(root.shape, TopAbs_FACE, faces);
    const auto ais = Mayo::OccHandle<XCAFPrs_AISObject>::DownCast(root.graphics);
    QJsonArray faceRecords;
    int hiddenCount = 0;
    int triangulatedCount = 0;
    int boundarylessCount = 0;
    for (int i = 1; i <= faces.Extent(); ++i) {
        const TopoDS_Face& face = TopoDS::Face(faces(i));
        const bool hasWire = TopExp_Explorer(face, TopAbs_WIRE).More();
        TopLoc_Location location;
        const auto mesh = BRep_Tool::Triangulation(face, location);
        const auto surface = BRep_Tool::Surface(face, location);
        const bool triangulated = !mesh.IsNull() && mesh->NbTriangles() > 0;
        bool hidden = false;
        if (ais && ais->CustomAspectsMap().IsBound(face))
            hidden = ais->CustomAspectsMap().Find(face)->IsHidden();

        hiddenCount += hidden;
        triangulatedCount += triangulated;
        boundarylessCount += !hasWire;
        QJsonObject detail{
            {"index", i}, {"has_wire", hasWire}, {"triangulated", triangulated},
            {"hidden_by_face_style", hidden}
        };
        if (!surface.IsNull()) {
            detail["surface"] = surface->DynamicType()->Name();
            double uMin, uMax, vMin, vMax;
            BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);
            detail["uv_bounds"] = QJsonArray{number(uMin), number(uMax), number(vMin), number(vMax)};
        }
        else {
            detail["surface"] = QJsonValue();
        }

        faceRecords.append(detail);
    }

    TCollection_AsciiString labelEntry;
    TDF_Tool::Entry(root.label, labelEntry);
    return {
        {"label", labelEntry.ToCString()}, {"face_count", faces.Extent()},
        {"boundaryless_face_count", boundarylessCount},
        {"triangulated_face_count", triangulatedCount},
        {"hidden_face_style_count", hiddenCount}, {"faces", faceRecords},
        {"graphics_box", boxRecord(Mayo::GraphicsUtils::AisObject_boundingBox(root.graphics))},
        {"graphics_keeps_original_shape", ais && ais->Shape().IsSame(root.shape)}
    };
}

void setFixedCamera(const Mayo::OccHandle<V3d_View>& view)
{
    // Both builds use exactly the same camera and clipping planes. Do not derive
    // this camera from the suspect geometry's bounding box.
    view->SetAutoZFitMode(false);
    auto camera = view->Camera();
    camera->SetProjectionType(Graphic3d_Camera::Projection_Orthographic);
    camera->SetEye(gp_Pnt(200., -200., 200.));
    camera->SetCenter(gp_Pnt(15., 15., 15.));
    camera->SetUp(gp_Dir(0., 0., 1.));
    camera->SetScale(150.);
    camera->SetZRange(0.1, 2000.);
}

QJsonObject cameraRecord(const Mayo::OccHandle<V3d_View>& view)
{
    const auto camera = view->Camera();
    const gp_Pnt eye = camera->Eye();
    const gp_Pnt center = camera->Center();
    return {
        {"scale", number(camera->Scale())}, {"distance", number(camera->Distance())},
        {"eye", coordinates(eye.X(), eye.Y(), eye.Z())},
        {"center", coordinates(center.X(), center.Y(), center.Z())},
        {"z_near", number(camera->ZNear())}, {"z_far", number(camera->ZFar())}
    };
}

QJsonObject capture(
        const char* name,
        const std::vector<Root>& roots,
        const Mayo::OccHandle<V3d_View>& view,
        const QDir& output
    )
{
    QJsonObject record{{"name", name}};
    Bnd_Box totalBox;
    QJsonArray rootRecords;
    for (const Root& root : roots) {
        rootRecords.append(topologyRecord(root));
        const auto box = Mayo::GraphicsUtils::AisObject_boundingBox(root.graphics);
        if (!box.IsVoid())
            totalBox.Add(box);
    }
    record["roots"] = rootRecords;
    record["total_graphics_box"] = boxRecord(totalBox);

    setFixedCamera(view);
    const QString fixedName = QString::fromLatin1(name) + "_fixed.png";
    record["fixed_camera"] = cameraRecord(view);
    record["fixed_image"] = fixedName;
    record["fixed_image_written"] = view->Dump(output.filePath(fixedName).toUtf8().constData());

    // This overload is the production GUI's bounding-box-driven fit operation.
    if (!totalBox.IsVoid()) {
        Mayo::GraphicsUtils::V3dView_fitAll(view, totalBox);
        view->ZFitAll();
    }
    record["fit_applied"] = !totalBox.IsVoid();
    record["fit_camera"] = cameraRecord(view);
    const QString fitName = QString::fromLatin1(name) + "_fit.png";
    record["fit_image"] = fitName;
    record["fit_image_written"] = view->Dump(output.filePath(fitName).toUtf8().constData());
    return record;
}

std::vector<Mode> modesFor(const std::string& name)
{
    if (name == "all") {
        return {
            {"shaded_edges", GraphicsShapeObjectDriver::DisplayMode_ShadedWithFaceBoundary},
            {"shaded", GraphicsShapeObjectDriver::DisplayMode_Shaded},
            {"wireframe", GraphicsShapeObjectDriver::DisplayMode_Wireframe},
            {"shaded_edges_restored", GraphicsShapeObjectDriver::DisplayMode_ShadedWithFaceBoundary}
        };
    }
    if (name == "shaded_edges")
        return {{"shaded_edges", GraphicsShapeObjectDriver::DisplayMode_ShadedWithFaceBoundary}};
    if (name == "shaded")
        return {{"shaded", GraphicsShapeObjectDriver::DisplayMode_Shaded}};
    if (name == "wireframe")
        return {{"wireframe", GraphicsShapeObjectDriver::DisplayMode_Wireframe}};

    throw std::runtime_error("Unknown display mode: " + name);
}

int inspect(const QString& filename, const QDir& output, const std::vector<Mode>& modes)
{
    auto app = Mayo::makeOccHandle<Mayo::Application>();
    app->setAutoExpandCompoundToAssembly(false);
    auto document = app->newDocument();
    const QString suffix = QFileInfo(filename).suffix().toLower();
    if (suffix == "brep") {
        TopoDS_Shape shape;
        BRep_Builder builder;
        if (!BRepTools::Read(shape, filename.toUtf8().constData(), builder))
            throw std::runtime_error("Cannot read BREP");

        document->xcaf().shapeTool()->AddShape(shape, false);
    }
    else if (suffix == "step" || suffix == "stp") {
        STEPCAFControl_Reader reader;
        reader.SetColorMode(true);
        reader.SetNameMode(true);
        reader.SetLayerMode(true);
        reader.SetPropsMode(true);
        if (reader.ReadFile(filename.toUtf8().constData()) != IFSelect_RetDone)
            throw std::runtime_error("Cannot read STEP");

        Mayo::OccHandle<TDocStd_Document> baseDocument = document;
        if (!reader.Transfer(baseDocument))
            throw std::runtime_error("Cannot transfer STEP");
    }
    else {
        throw std::runtime_error("Expected .step, .stp, or .brep input");
    }

    QJsonObject report{
        {"input", QFileInfo(filename).absoluteFilePath()}, {"occt_version", OCC_VERSION_COMPLETE},
        {"scope", "Production Mayo GraphicsShapeObjectDriver with root XCAF presentations"},
        {"meshing", "BRepMesh_IncrementalMesh: absolute deflection 0.1, angle 0.35, serial"},
        {"brep_comparison", "Full BREP, including triangulations, immediately before and after display"}
    };

    std::vector<Root> roots;
    for (const TDF_Label& label : document->xcaf().topLevelFreeShapes()) {
        Root root;
        root.label = label;
        root.shape = Mayo::XCaf::shape(label);
        // Match Mayo's import-then-mesh-then-display sequence. The diagnostic uses
        // an explicit finite deflection so this step does not depend on any fit.
        BRepMesh_IncrementalMesh mesh(root.shape, 0.1, false, 0.35, false);
        root.beforeBrep = serialize(root.shape);
        roots.push_back(std::move(root));
    }
    if (roots.empty())
        throw std::runtime_error("The input has no free XCAF shapes");

    Mayo::GraphicsScene scene;
    Mayo::IO::ImageWriter::Parameters imageParameters;
    imageParameters.width = 1280;
    imageParameters.height = 960;
    imageParameters.backgroundColorStart = Quantity_Color(0.94, 0.95, 0.97, Quantity_TOC_RGB);
    imageParameters.msaaSamples = Mayo::IO::ImageWriter::MsaaSamples::Off;
    auto view = Mayo::IO::ImageWriter::createV3dView(&scene, imageParameters);
    auto driver = Mayo::makeOccHandle<GraphicsShapeObjectDriver>();
    for (Root& root : roots) {
        root.graphics = driver->createObject(root.label);
        if (!root.graphics)
            throw std::runtime_error("The production Mayo shape driver rejected a root shape");

        scene.addObject(root.graphics);
    }

    QJsonArray presentations;
    for (const Mode& mode : modes) {
        for (Root& root : roots)
            driver->applyDisplayMode(root.graphics, mode.value);

        presentations.append(capture(mode.name, roots, view, output));
    }

    // Rebuild XCAF styles, then recompute all existing display presentations.
    // The patched policy must survive this real refresh, not just construction.
    for (Root& root : roots) {
        auto ais = Mayo::OccHandle<XCAFPrs_AISObject>::DownCast(root.graphics);
        ais->DispatchStyles();
        ais->Redisplay(true);
    }
    presentations.append(capture("styles_refreshed", roots, view, output));
    report["presentations"] = presentations;

    bool unchanged = true;
    QJsonArray preservation;
    for (std::size_t i = 0; i < roots.size(); ++i) {
        const Root& root = roots[i];
        const std::string after = serialize(root.shape);
        const bool equal = after == root.beforeBrep;
        unchanged = unchanged && equal;
        preservation.append(QJsonObject{
            {"root", static_cast<int>(i + 1)}, {"bytes_equal", equal},
            {"before_sha256", sha256(root.beforeBrep)}, {"after_sha256", sha256(after)},
            {"before_bytes", static_cast<double>(root.beforeBrep.size())},
            {"after_bytes", static_cast<double>(after.size())}
        });
        const QString stem = "root_" + QString::number(static_cast<qulonglong>(i + 1));
        writeBytes(output.filePath(stem + "_before.brep"), QByteArray::fromStdString(root.beforeBrep));
        writeBytes(output.filePath(stem + "_after.brep"), QByteArray::fromStdString(after));
    }
    report["geometry_unchanged"] = unchanged;
    report["preservation"] = preservation;
    writeBytes(output.filePath("stats.json"), QJsonDocument(report).toJson(QJsonDocument::Indented));
    std::cout << "Wrote " << output.filePath("stats.json").toStdString()
              << "; display preserved full BREP bytes: " << (unchanged ? "yes" : "NO") << '\n';
    return unchanged ? 0 : 3;
}

} // namespace

int main(int argc, char** argv)
{
    QCoreApplication qtApp(argc, argv);
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: inspect-infinite-faces input.step output-dir "
                     "[shaded_edges|shaded|wireframe|all]\n";
        return 2;
    }

    try {
        QDir output(QString::fromLocal8Bit(argv[2]));
        if (!output.mkpath("."))
            throw std::runtime_error("Cannot create the output directory");

        return inspect(QString::fromLocal8Bit(argv[1]), output,
                       modesFor(argc == 4 ? argv[3] : "shaded_edges"));
    }
    catch (const Standard_Failure& failure) {
        std::cerr << "OCCT failure: " << failure.GetMessageString() << '\n';
    }
    catch (const std::exception& failure) {
        std::cerr << "Failure: " << failure.what() << '\n';
    }
    return 1;
}
