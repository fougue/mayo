/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "document_fixture.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <Quantity_Color.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TDataStd_Name.hxx>
#include <TopExp_Explorer.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_Material.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_Volume.hxx>

#include <cmath>

namespace Mayo {

namespace {

TDF_Label addBox(const DocumentPtr& doc, double dx, double dy, double dz)
{
    return doc->xcaf().shapeTool()->AddShape(BRepPrimAPI_MakeBox(dx, dy, dz).Shape(), false);
}

} // namespace

void TestBase::XCaf_isNull_test()
{
    DocumentFixture fixture;
    QVERIFY(!fixture.doc->xcaf().isNull());
}

void TestBase::XCaf_topLevelFreeShapes_test()
{
    DocumentFixture fixture;
    XCaf& xcaf = fixture.doc->xcaf();

    QVERIFY(xcaf.topLevelFreeShapes().IsEmpty());

    const TDF_Label firstShape = addBox(fixture.doc, 1., 2., 3.);
    const TDF_Label secondShape = addBox(fixture.doc, 2., 3., 4.);
    const NCollection_Sequence<TDF_Label> freeShapes = xcaf.topLevelFreeShapes();

    QCOMPARE(freeShapes.Size(), 2);
    QCOMPARE(freeShapes.Value(1), firstShape);
    QCOMPARE(freeShapes.Value(2), secondShape);
}

void TestBase::XCaf_shapeSubs_test()
{
    DocumentFixture fixture;
    const TDF_Label shapeLabel = addBox(fixture.doc, 1., 2., 3.);

    QVERIFY(XCaf::shapeSubs(shapeLabel).IsEmpty());

    const TopoDS_Shape box = XCaf::shape(shapeLabel);
    TopExp_Explorer faceExplorer(box, TopAbs_FACE);
    QVERIFY(faceExplorer.More());
    const TDF_Label faceLabel = fixture.doc->xcaf().shapeTool()->AddSubShape(
        shapeLabel, faceExplorer.Current()
    );

    const NCollection_Sequence<TDF_Label> subShapes = XCaf::shapeSubs(shapeLabel);
    QCOMPARE(subShapes.Size(), 1);
    QCOMPARE(subShapes.First(), faceLabel);
}

void TestBase::XCaf_shapeColor_test()
{
    DocumentFixture fixture;
    const TDF_Label noColorLabel = addBox(fixture.doc, 1., 1., 1.);
    const TDF_Label genericColorLabel = addBox(fixture.doc, 2., 2., 2.);
    const TDF_Label surfaceColorLabel = addBox(fixture.doc, 3., 3., 3.);
    const TDF_Label curveColorLabel = addBox(fixture.doc, 4., 4., 4.);

    const Quantity_Color genericColor{0.8, 0.1, 0.2, Quantity_TOC_RGB};
    const Quantity_Color surfaceColor{0.1, 0.8, 0.2, Quantity_TOC_RGB};
    const Quantity_Color curveColor{0.1, 0.2, 0.8, Quantity_TOC_RGB};
    auto colorTool = fixture.doc->xcaf().colorTool();
    colorTool->SetColor(genericColorLabel, genericColor, XCAFDoc_ColorGen);
    colorTool->SetColor(surfaceColorLabel, surfaceColor, XCAFDoc_ColorSurf);
    colorTool->SetColor(curveColorLabel, curveColor, XCAFDoc_ColorCurv);

    const auto& xcaf = fixture.doc->xcaf();
    QVERIFY(!xcaf.hasShapeColor(noColorLabel));
    QVERIFY(xcaf.hasShapeColor(genericColorLabel));
    QVERIFY(xcaf.hasShapeColor(surfaceColorLabel));
    QVERIFY(xcaf.hasShapeColor(curveColorLabel));
    QVERIFY(xcaf.shapeColor(genericColorLabel).IsEqual(genericColor));
    QVERIFY(xcaf.shapeColor(surfaceColorLabel).IsEqual(surfaceColor));
    QVERIFY(xcaf.shapeColor(curveColorLabel).IsEqual(curveColor));
}

void TestBase::XCaf_validationProperties_test()
{
    DocumentFixture fixture;
    const TDF_Label label = fixture.doc->rootLabel().NewChild();

    const XCaf::ValidationProperties emptyProps = XCaf::validationProperties(label);
    QVERIFY(!emptyProps.hasCentroid);
    QVERIFY(!emptyProps.hasArea);
    QVERIFY(!emptyProps.hasVolume);

    const gp_Pnt centroid{1.25, -2.5, 3.75};
    XCAFDoc_Centroid::Set(label, centroid);
    XCAFDoc_Area::Set(label, 12.5);
    XCAFDoc_Volume::Set(label, 42.75);

    const XCaf::ValidationProperties props = XCaf::validationProperties(label);
    QVERIFY(props.hasCentroid);
    QVERIFY(props.hasArea);
    QVERIFY(props.hasVolume);
    QVERIFY(props.centroid.IsEqual(centroid, 1e-12));
    QCOMPARE(props.area.value(), 12.5);
    QCOMPARE(props.volume.value(), 42.75);
}

void TestBase::XCaf_diffTopLevelFreeShapes_test()
{
    DocumentFixture fixture;
    auto& xcaf = fixture.doc->xcaf();

    const TDF_Label firstShape = addBox(fixture.doc, 1., 1., 1.);
    const NCollection_Sequence<TDF_Label> beforeSecond = xcaf.topLevelFreeShapes();
    const TDF_Label secondShape = addBox(fixture.doc, 2., 2., 2.);

    const NCollection_Sequence<TDF_Label> diff = xcaf.diffTopLevelFreeShapes(beforeSecond);
    QCOMPARE(diff.Size(), 1);
    QCOMPARE(diff.First(), secondShape);

    const NCollection_Sequence<TDF_Label> current = xcaf.topLevelFreeShapes();
    QVERIFY(xcaf.diffTopLevelFreeShapes(current).IsEmpty());
    QCOMPARE(current.Size(), 2);
    QCOMPARE(current.First(), firstShape);
}

void TestBase::XCaf_shapeMaterial_test()
{
    DocumentFixture fixture;
    const TDF_Label shapeLabel = addBox(fixture.doc, 1., 2., 3.);

    QVERIFY(XCaf::shapeMaterial(shapeLabel).IsNull());

    auto materialTool = fixture.doc->xcaf().materialTool();
    materialTool->SetMaterial(
        shapeLabel,
        makeOccHandle<TCollection_HAsciiString>("Aluminum"),
        makeOccHandle<TCollection_HAsciiString>("Test material"),
        2.7,
        makeOccHandle<TCollection_HAsciiString>("g/cm^3"),
        makeOccHandle<TCollection_HAsciiString>("mass density")
    );

    const auto material = XCaf::shapeMaterial(shapeLabel);
    QVERIFY(!material.IsNull());
    QCOMPARE(material->GetDensity(), 2.7);
}

void TestBase::XCaf_userDefinedAttributes_test()
{
    DocumentFixture fixture;
    XCaf& xcaf = fixture.doc->xcaf();

    // Must not crash for null labels
    QVERIFY(xcaf.shapeUserDefinedAttributes(TDF_Label{}).IsNull());

    // Must not create user defined attributes if none attached
    const TDF_Label shapeLabel = fixture.doc->newEntityShapeLabel();
    TDataStd_Name::Set(shapeLabel, L"Shape1");
    fixture.doc->addEntityTreeNode(shapeLabel);
    QVERIFY(xcaf.shapeUserDefinedAttributes(shapeLabel).IsNull());
}

} // namespace Mayo
