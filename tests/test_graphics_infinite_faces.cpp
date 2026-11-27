/****************************************************************************
** Copyright (c) 2026, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_graphics.h"

#include "../src/base/application.h"
#include "../src/base/brep_utils.h"
#include "../src/base/document.h"
#include "../src/base/math_utils.h"
#include "../src/base/mesh_utils.h"
#include "../src/graphics/graphics_scene.h"
#include "../src/graphics/graphics_shape_object_driver.h"
#include "../src/graphics/graphics_utils.h"

#include <AIS_ConnectedInteractive.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Graphic3d_MapOfStructure.hxx>
#include <Graphic3d_Structure.hxx>
#include <Graphic3d_StructureManager.hxx>
#include <Precision.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <PrsMgr_Presentation.hxx>
#include <Select3D_SensitiveEntity.hxx>
#include <SelectMgr_Selection.hxx>
#include <SelectMgr_SensitiveEntity.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <XCAFPrs_AISObject.hxx>

#include <QtTest/QtTest>

#include <array>
#include <cmath>
#include <sstream>

namespace Mayo {
namespace {

TopoDS_Face makeBoundarylessFace(const OccHandle<Geom_Surface>& surface)
{
    // MakeFace(surface) from BRepBuilderAPI can add natural boundary wires.
    // BRep_Builder preserves the empty STEP ADVANCED_FACE bounds list exactly.
    TopoDS_Face face;
    BRep_Builder().MakeFace(face, surface, Precision::Confusion());
    return face;
}

OccHandle<Geom_Surface> makeUnboundedSurface(int kind)
{
    const gp_Ax3 axis(gp_Pnt(5, 10, 15), gp::DZ());
    switch (kind) {
    case 0: return new Geom_CylindricalSurface(axis, 3.);
    case 1: return new Geom_Plane(axis);
    default: return new Geom_ConicalSurface(axis, 0.3, 3.);
    }
}

TopoDS_Shape makeMeshedBox()
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10, 20, 30);
    BRepMesh_IncrementalMesh mesh(box, 0.1);
    return box;
}

TopoDS_Compound makeCompound(const TopoDS_Shape& first, const TopoDS_Shape& second)
{
    TopoDS_Compound compound = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&compound, first);
    BRepUtils::addShape(&compound, second);
    return compound;
}

QByteArray shapeBytes(const TopoDS_Shape& shape)
{
    std::ostringstream stream;
    BRepTools::Write(shape, stream);
    return QByteArray::fromStdString(stream.str());
}

bool isHidden(const OccHandle<XCAFPrs_AISObject>& object, const TopoDS_Shape& shape)
{
    OccHandle<AIS_ColoredDrawer> drawer;
    return object->CustomAspectsMap().Find(shape, drawer) && drawer->IsHidden();
}

bool boxMatches(const Bnd_Box& box, const std::array<double, 6>& expected)
{
    if (box.IsVoid() || box.IsOpen())
        return false;

    std::array<double, 6> bounds;
    box.Get(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
    for (size_t i = 0; i < bounds.size(); ++i) {
        if (!std::isfinite(bounds[i]) || std::abs(bounds[i] - expected[i]) > 1e-3)
            return false;
    }

    return true;
}

QString boxDescription(const Bnd_Box& box)
{
    if (box.IsVoid())
        return "void";
    if (box.IsOpen())
        return "open";

    std::array<double, 6> bounds;
    box.Get(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
    return QString("[%1, %2, %3]..[%4, %5, %6]")
        .arg(bounds[0]).arg(bounds[1]).arg(bounds[2])
        .arg(bounds[3]).arg(bounds[4]).arg(bounds[5]);
}

Bnd_Box sensitiveBounds(const OccHandle<SelectMgr_Selection>& selection)
{
    Bnd_Box box;
    for (const auto& sensitive : selection->Entities()) {
        const auto bounds = sensitive->BaseSensitive()->BoundingBox();
        if (bounds.IsValid()) {
            const auto min = bounds.CornerMin();
            const auto max = bounds.CornerMax();
            box.Add(gp_Pnt(min.x(), min.y(), min.z()));
            box.Add(gp_Pnt(max.x(), max.y(), max.z()));
        }
    }
    return box;
}

struct ShapeFixture {
    ApplicationPtr app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    GraphicsShapeObjectDriverPtr driver = makeOccHandle<GraphicsShapeObjectDriver>();
    TDF_Label label;
    OccHandle<XCAFPrs_AISObject> object;

    explicit ShapeFixture(const TopoDS_Shape& shape)
    {
        label = doc->newEntityShapeLabel();
        doc->xcaf().setShape(label, shape);
        object = OccHandle<XCAFPrs_AISObject>::DownCast(driver->createObject(label));
    }
};

} // namespace

void TestGraphics::ShapeObject_unboundedFaces_test_data()
{
    QTest::addColumn<int>("surfaceKind");
    QTest::newRow("cylinder") << 0;
    QTest::newRow("plane") << 1;
    QTest::newRow("cone") << 2;
}

void TestGraphics::ShapeObject_unboundedFaces_test()
{
    QFETCH(int, surfaceKind);
    const TopoDS_Face unbounded = makeBoundarylessFace(makeUnboundedSurface(surfaceKind));
    QVERIFY(!TopExp_Explorer(unbounded, TopAbs_WIRE).More());
    TopLoc_Location location;
    QVERIFY(BRep_Tool::Triangulation(unbounded, location).IsNull());
    const TopoDS_Shape shape = makeCompound(makeMeshedBox(), unbounded);
    const QByteArray before = shapeBytes(shape);
    ShapeFixture fixture(shape);
    QVERIFY(fixture.object);
    GraphicsScene scene;
    scene.addObject(fixture.object);

    // Include a return to both shaded modes after wireframe, and force full style
    // synchronization. A one-off CustomAspects(face)->SetHidden() is insufficient.
    for (auto mode : {
             GraphicsShapeObjectDriver::DisplayMode_Shaded,
             GraphicsShapeObjectDriver::DisplayMode_ShadedWithFaceBoundary,
             GraphicsShapeObjectDriver::DisplayMode_Wireframe,
             GraphicsShapeObjectDriver::DisplayMode_Shaded,
             GraphicsShapeObjectDriver::DisplayMode_ShadedWithFaceBoundary})
    {
        fixture.driver->applyDisplayMode(fixture.object, mode);
        fixture.object->DispatchStyles(true);
        fixture.object->Redisplay(true);
        QVERIFY(isHidden(fixture.object, unbounded));
        QVERIFY(boxMatches(GraphicsUtils::AisObject_boundingBox(fixture.object), {0, 0, 0, 10, 20, 30}));
        QCOMPARE(fixture.driver->currentDisplayMode(fixture.object), int(mode));
        QVERIFY(fixture.object->Shape().IsSame(shape));
        QCOMPARE(shapeBytes(XCaf::shape(fixture.label)), before);
    }
}

void TestGraphics::ShapeObject_retainedFaces_test_data()
{
    QTest::addColumn<int>("faceKind");
    QTest::newRow("sphere-natural-domain") << 0;
    QTest::newRow("torus-natural-domain") << 1;
    QTest::newRow("bezier-natural-domain") << 2;
    QTest::newRow("trimmed-surface-natural-domain") << 3;
    QTest::newRow("bounded-cylinder") << 4;
    QTest::newRow("bounded-plane") << 5;
    QTest::newRow("mesh-only-face") << 6;
    QTest::newRow("unbounded-surface-with-triangulation") << 7;
}

void TestGraphics::ShapeObject_retainedFaces_test()
{
    QFETCH(int, faceKind);
    TopoDS_Face face;
    const gp_Ax3 axis(gp_Pnt(0, 0, 0), gp::DZ());
    switch (faceKind) {
    case 0:
        face = makeBoundarylessFace(new Geom_SphericalSurface(axis, 3.));
        break;
    case 1:
        face = makeBoundarylessFace(new Geom_ToroidalSurface(axis, 5., 1.));
        break;
    case 2: {
        TColgp_Array2OfPnt poles(1, 2, 1, 2);
        poles(1, 1) = gp_Pnt(0, 0, 0);
        poles(1, 2) = gp_Pnt(0, 10, 0);
        poles(2, 1) = gp_Pnt(10, 0, 2);
        poles(2, 2) = gp_Pnt(10, 10, 2);
        face = makeBoundarylessFace(new Geom_BezierSurface(poles));
        break;
    }
    case 3:
        face = makeBoundarylessFace(new Geom_RectangularTrimmedSurface(
            new Geom_Plane(axis), -5., 5., -7., 7.
        ));
        break;
    case 4:
        face = BRepBuilderAPI_MakeFace(
            new Geom_CylindricalSurface(axis, 3.), 0., 2. * MathConst::pi, -5., 5., Precision::Confusion()
        );
        break;
    case 5:
        // Deliberately large finite geometry: world-coordinate thresholds must
        // not turn a normal bounded face into a suppressed face.
        face = BRepBuilderAPI_MakeFace(
            new Geom_Plane(axis), -1e8, 1e8, -1e8, 1e8, Precision::Confusion()
        );
        break;
    default: {
        auto mesh = makeOccHandle<Poly_Triangulation>(3, 1, false);
        MeshUtils::setNode(mesh, 1, gp_Pnt(0, 0, 0));
        MeshUtils::setNode(mesh, 2, gp_Pnt(10, 0, 0));
        MeshUtils::setNode(mesh, 3, gp_Pnt(0, 10, 0));
        MeshUtils::setTriangle(mesh, 1, Poly_Triangle(1, 2, 3));
        if (faceKind == 6) {
            BRep_Builder().MakeFace(face, mesh);
        }
        else {
            face = makeBoundarylessFace(new Geom_Plane(axis));
            BRep_Builder().UpdateFace(face, mesh);
        }
        break;
    }
    }

    ShapeFixture fixture(face);
    QVERIFY(fixture.object);
    fixture.object->DispatchStyles();
    QVERIFY(!isHidden(fixture.object, face));
    QVERIFY(fixture.object->Shape().IsSame(face));

    if (faceKind >= 6) {
        TopLoc_Location location;
        const auto mesh = BRep_Tool::Triangulation(face, location);
        GraphicsScene scene;
        scene.addObject(fixture.object);
        QVERIFY(boxMatches(GraphicsUtils::AisObject_boundingBox(fixture.object), {0, 0, 0, 10, 10, 0}));
        // OCCT may compute normals and cache face flags during display. The
        // original mesh, its coordinates and connectivity must stay intact.
        QCOMPARE(BRep_Tool::Triangulation(face, location), mesh);
        QCOMPARE(mesh->NbNodes(), 3);
        QCOMPARE(mesh->NbTriangles(), 1);
        QVERIFY(mesh->Node(1).IsEqual(gp_Pnt(0, 0, 0), 0.));
        QVERIFY(mesh->Node(2).IsEqual(gp_Pnt(10, 0, 0), 0.));
        QVERIFY(mesh->Node(3).IsEqual(gp_Pnt(0, 10, 0), 0.));
        int first, second, third;
        mesh->Triangle(1).Get(first, second, third);
        QCOMPARE(first, 1);
        QCOMPARE(second, 2);
        QCOMPARE(third, 3);
    }

    // Independent edges and wires still use the normal presentation path.
    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(gp_Pnt(0, 0, 0), gp_Pnt(20, 0, 0));
    const TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge);
    ShapeFixture wireFixture(makeCompound(face, wire));
    wireFixture.object->DispatchStyles();
    QVERIFY(!isHidden(wireFixture.object, wire));
    QVERIFY(!isHidden(wireFixture.object, edge));

    if (faceKind == 0) {
        ShapeFixture standaloneWireFixture(wire);
        GraphicsScene scene;
        scene.addObject(standaloneWireFixture.object);
        for (auto mode : {
                 GraphicsShapeObjectDriver::DisplayMode_ShadedWithFaceBoundary,
                 GraphicsShapeObjectDriver::DisplayMode_Wireframe})
        {
            standaloneWireFixture.driver->applyDisplayMode(standaloneWireFixture.object, mode);
            QVERIFY(boxMatches(GraphicsUtils::AisObject_boundingBox(standaloneWireFixture.object), {0, 0, 0, 20, 0, 0}));
        }
    }
}

void TestGraphics::ShapeObject_unboundedFaceStylesAndGeometry_test()
{
    const TopoDS_Shape box = makeMeshedBox();
    const TopoDS_Face unbounded = makeBoundarylessFace(makeUnboundedSurface(0));
    const TopoDS_Compound shape = makeCompound(box, unbounded);
    ShapeFixture fixture(shape);
    TopExp_Explorer faces(box, TopAbs_FACE);
    const TopoDS_Face redFace = TopoDS::Face(faces.Current());
    faces.Next();
    const TopoDS_Face invisibleFace = TopoDS::Face(faces.Current());
    const auto shapeTool = fixture.doc->xcaf().shapeTool();
    const auto colorTool = fixture.doc->xcaf().colorTool();
    const TDF_Label redLabel = shapeTool->AddSubShape(fixture.label, redFace);
    const TDF_Label invisibleLabel = shapeTool->AddSubShape(fixture.label, invisibleFace);
    QVERIFY(!redLabel.IsNull());
    QVERIFY(!invisibleLabel.IsNull());
    const Quantity_Color red(Quantity_NOC_RED);
    colorTool->SetColor(redLabel, red, XCAFDoc_ColorSurf);
    colorTool->SetVisibility(invisibleLabel, false);
    const QByteArray before = shapeBytes(shape);

    GraphicsScene scene;
    scene.addObject(fixture.object);
    for (int iteration = 0; iteration < 2; ++iteration) {
        fixture.object->DispatchStyles(true);
        fixture.object->Redisplay(true);
        QVERIFY(isHidden(fixture.object, unbounded));
        QVERIFY(isHidden(fixture.object, invisibleFace));
        QVERIFY(!isHidden(fixture.object, redFace));
        OccHandle<AIS_ColoredDrawer> redDrawer;
        QVERIFY(fixture.object->CustomAspectsMap().Find(redFace, redDrawer));
        QVERIFY(redDrawer->ShadingAspect()->Color().IsEqual(red));
        QVERIFY(colorTool->IsVisible(fixture.label));
        QVERIFY(colorTool->IsVisible(redLabel));
        QVERIFY(!colorTool->IsVisible(invisibleLabel));
        QCOMPARE(shapeBytes(XCaf::shape(fixture.label)), before);
    }
}

void TestGraphics::ShapeObject_unboundedFaceSelection_test()
{
    const TopoDS_Face unbounded = makeBoundarylessFace(makeUnboundedSurface(0));
    ShapeFixture fixture(makeCompound(makeMeshedBox(), unbounded));
    GraphicsScene scene;
    scene.addObject(fixture.object);

    // Whole-object picking retains the original model owner but only finite
    // visible geometry contributes sensitive entities.
    const auto& selection = fixture.object->Selection(0);
    QVERIFY(!selection.IsNull());
    QVERIFY(!selection->IsEmpty());
    for (const auto& sensitive : selection->Entities()) {
        const auto bounds = sensitive->BaseSensitive()->BoundingBox();
        if (!bounds.IsValid())
            continue;

        const auto min = bounds.CornerMin();
        const auto max = bounds.CornerMax();
        for (int axis = 0; axis < 3; ++axis) {
            QVERIFY(std::isfinite(min[axis]));
            QVERIFY(std::isfinite(max[axis]));
            QVERIFY(min[axis] >= -1e-3);
            QVERIFY(max[axis] <= (axis + 1) * 10. + 1e-3);
        }
    }

    scene.activateObjectSelection(fixture.object, AIS_Shape::SelectionMode(TopAbs_FACE));
    int faceOwnerCount = 0;
    scene.foreachOwner(fixture.object, AIS_Shape::SelectionMode(TopAbs_FACE), [&](const GraphicsOwnerPtr& owner) {
        const auto brepOwner = OccHandle<StdSelect_BRepOwner>::DownCast(owner);
        QVERIFY(brepOwner);
        QVERIFY(brepOwner->HasShape());
        QVERIFY(!brepOwner->Shape().IsSame(unbounded));
        QCOMPARE(brepOwner->Shape().ShapeType(), TopAbs_FACE);
        ++faceOwnerCount;
    });
    QCOMPARE(faceOwnerCount, 6);
}

void TestGraphics::ShapeObject_unboundedFaceInstances_test()
{
    const TopoDS_Face unbounded = makeBoundarylessFace(makeUnboundedSurface(0));
    ShapeFixture fixture(makeCompound(makeMeshedBox(), unbounded));
    const auto shapeTool = fixture.doc->xcaf().shapeTool();
    const TDF_Label assemblyLabel = fixture.doc->newEntityShapeLabel();
    fixture.doc->xcaf().setShape(assemblyLabel, BRepUtils::makeEmptyCompound());
    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(100, 0, 0));
    const TDF_Label firstComponent = shapeTool->AddComponent(assemblyLabel, fixture.label, TopLoc_Location());
    const TDF_Label secondComponent = shapeTool->AddComponent(assemblyLabel, fixture.label, TopLoc_Location(translation));
    shapeTool->UpdateAssemblies();
    const TopoDS_Shape assemblyShape = XCaf::shape(assemblyLabel);
    const QByteArray before = shapeBytes(assemblyShape);
    auto assemblyObject = fixture.driver->createObject(assemblyLabel);
    GraphicsScene scene;
    scene.addObject(assemblyObject);
    QVERIFY(boxMatches(GraphicsUtils::AisObject_boundingBox(assemblyObject), {0, 0, 0, 110, 20, 30}));
    QVERIFY(XCaf::shapeReferred(firstComponent) == fixture.label);
    QVERIFY(XCaf::shapeReferred(secondComponent) == fixture.label);
    QCOMPARE(XCaf::shapeComponents(assemblyLabel).Length(), 2);
    QCOMPARE(shapeBytes(XCaf::shape(assemblyLabel)), before);

    // Mayo normally presents repeated components as connected AIS instances.
    auto instance = GraphicsUtils::AisObject_createConnected(fixture.object, TopLoc_Location(translation));
    instance->SetDisplayMode(AIS_Shaded);
    scene.addObject(instance);
    // No view is active in this test to refresh OCCT's cached instance bounds
    // after its presentation receives the local transformation.
    for (const auto& presentation : instance->Presentations())
        presentation->CalculateBoundBox();

    const Bnd_Box instanceBox = GraphicsUtils::AisObject_boundingBox(instance);
    QVERIFY2(boxMatches(instanceBox, {100, 0, 0, 110, 20, 30}), qPrintable(boxDescription(instanceBox)));
}

void TestGraphics::ShapeObject_unboundedFaceCompoundSelection_test_data()
{
    QTest::addColumn<int>("surfaceKind");
    QTest::addColumn<bool>("connected");
    QTest::addColumn<int>("rootType");
    QTest::newRow("cylinder-direct") << 0 << false << int(TopAbs_COMPOUND);
    QTest::newRow("cylinder-instance") << 0 << true << int(TopAbs_COMPOUND);
    QTest::newRow("plane-direct") << 1 << false << int(TopAbs_COMPOUND);
    QTest::newRow("plane-instance") << 1 << true << int(TopAbs_COMPOUND);
    QTest::newRow("cone-direct") << 2 << false << int(TopAbs_COMPOUND);
    QTest::newRow("cone-instance") << 2 << true << int(TopAbs_COMPOUND);
    QTest::newRow("plane-shell-direct") << 1 << false << int(TopAbs_SHELL);
    QTest::newRow("plane-shell-instance") << 1 << true << int(TopAbs_SHELL);
    QTest::newRow("plane-solid-direct") << 1 << false << int(TopAbs_SOLID);
    QTest::newRow("plane-solid-instance") << 1 << true << int(TopAbs_SOLID);
}

void TestGraphics::ShapeObject_unboundedFaceCompoundSelection_test()
{
    QFETCH(int, surfaceKind);
    QFETCH(bool, connected);
    QFETCH(int, rootType);
    const TopoDS_Face unbounded = makeBoundarylessFace(makeUnboundedSurface(surfaceKind));
    const TopoDS_Shape box = makeMeshedBox();
    TopoDS_Shape shape;
    if (rootType == TopAbs_COMPOUND) {
        shape = makeCompound(makeCompound(box, unbounded), BRepUtils::makeEmptyCompound());
    }
    else {
        // Preserve anomalous imported topology even when the root is a shell
        // or a solid. No sewing, healing or solid-validity assumptions are made.
        BRep_Builder builder;
        TopoDS_Shell shell;
        builder.MakeShell(shell);
        BRepUtils::forEachSubFace(box, [&](const TopoDS_Face& face) { builder.Add(shell, face); });
        builder.Add(shell, unbounded);
        if (rootType == TopAbs_SOLID) {
            TopoDS_Solid solid;
            builder.MakeSolid(solid);
            builder.Add(solid, shell);
            shape = solid;
        }
        else {
            shape = shell;
        }
    }
    QCOMPARE(int(shape.ShapeType()), rootType);
    ShapeFixture fixture(shape);
    GraphicsObjectPtr displayedObject = fixture.object;
    if (connected) {
        gp_Trsf translation;
        translation.SetTranslation(gp_Vec(100, 0, 0));
        auto instance = GraphicsUtils::AisObject_createConnected(fixture.object, TopLoc_Location(translation));
        instance->SetDisplayMode(AIS_Shaded);
        displayedObject = instance;
    }

    GraphicsScene scene;
    // Match the shaded selection highlight configured by MainWindow.
    scene.drawerHighlight(Prs3d_TypeOfHighlight_Selected)->SetDisplayMode(AIS_Shaded);
    scene.drawerHighlight(Prs3d_TypeOfHighlight_LocalSelected)->SetDisplayMode(AIS_Shaded);
    scene.addObject(displayedObject);
    scene.deactivateObjectSelection(displayedObject);
    const int selectionMode = AIS_Shape::SelectionMode(TopAbs_COMPOUND);
    scene.activateObjectSelection(displayedObject, selectionMode);
    QVERIFY(displayedObject->HasSelection(selectionMode));
    const auto& selection = displayedObject->Selection(selectionMode);
    QVERIFY(!selection.IsNull());
    QVERIFY(!selection->IsEmpty());
    const Bnd_Box pickingBox = sensitiveBounds(selection);

    int ownerCount = 0;
    Bnd_Box highlightedBox;
    scene.foreachOwner(displayedObject, selectionMode, [&](const GraphicsOwnerPtr& owner) {
        const auto brepOwner = OccHandle<StdSelect_BRepOwner>::DownCast(owner);
        QVERIFY(brepOwner);
        // Measurements still receive the original aggregate and its location.
        QVERIFY(brepOwner->Shape().IsSame(shape));
        QVERIFY(!brepOwner->ComesFromDecomposition());
        const gp_XYZ translation = brepOwner->Location().Transformation().TranslationPart();
        QCOMPARE(translation.X(), connected ? 100. : 0.);
        QCOMPARE(translation.Y(), 0.);
        QCOMPARE(translation.Z(), 0.);
        ++ownerCount;
        auto context = displayedObject->InteractiveContext();
        context->SetSelected(owner, false);
        Graphic3d_MapOfStructure highlighted;
        scene.v3dViewer()->StructureManager()->HighlightedStructures(highlighted);
        for (Graphic3d_MapOfStructure::Iterator it(highlighted); it.More(); it.Next())
            highlightedBox.Add(it.Value()->MinMaxValues(true));

        context->ClearSelected(false);
    });

    const auto expectedHighlightBox = connected
        ? std::array<double, 6>{100, 0, 0, 110, 20, 30}
        : std::array<double, 6>{0, 0, 0, 10, 20, 30};
    qInfo().noquote() << "Compound mode" << selectionMode << "connected" << connected
                     << "sensitive bounds" << boxDescription(pickingBox)
                     << "highlight bounds" << boxDescription(highlightedBox);
    QCOMPARE(ownerCount, 1);
    QVERIFY2(boxMatches(pickingBox, {0, 0, 0, 10, 20, 30}), qPrintable(boxDescription(pickingBox)));
    QVERIFY2(boxMatches(highlightedBox, expectedHighlightBox), qPrintable(boxDescription(highlightedBox)));
    QVERIFY(fixture.object->Shape().IsSame(shape));

    // Rebuild styles and selection after the first selection/highlight cycle.
    // Source and connected owners are recreated by OCCT on these updates.
    fixture.object->DispatchStyles(true);
    fixture.object->UpdateSelection();
    displayedObject->InteractiveContext()->Redisplay(displayedObject, false, true);
    displayedObject->InteractiveContext()->RecomputeSelectionOnly(displayedObject);
    const Bnd_Box refreshedPickingBox = sensitiveBounds(displayedObject->Selection(selectionMode));
    QVERIFY2(boxMatches(refreshedPickingBox, {0, 0, 0, 10, 20, 30}), qPrintable(boxDescription(refreshedPickingBox)));
    scene.foreachOwner(displayedObject, selectionMode, [&](const GraphicsOwnerPtr& owner) {
        const auto brepOwner = OccHandle<StdSelect_BRepOwner>::DownCast(owner);
        QVERIFY(brepOwner);
        QVERIFY(!brepOwner->ComesFromDecomposition());
        QVERIFY(brepOwner->Shape().IsSame(shape));
    });

    // Face picking remains available on both the product and its instance.
    const int faceMode = AIS_Shape::SelectionMode(TopAbs_FACE);
    scene.activateObjectSelection(displayedObject, faceMode);
    int faceOwnerCount = 0;
    scene.foreachOwner(displayedObject, faceMode, [&](const GraphicsOwnerPtr& owner) {
        const auto brepOwner = OccHandle<StdSelect_BRepOwner>::DownCast(owner);
        QVERIFY(brepOwner);
        QCOMPARE(brepOwner->Shape().ShapeType(), TopAbs_FACE);
        QVERIFY(!brepOwner->Shape().IsSame(unbounded));
        ++faceOwnerCount;
    });
    QCOMPARE(faceOwnerCount, 6);
}

} // namespace Mayo
