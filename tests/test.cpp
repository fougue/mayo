/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

// Need to include this first because of MSVC conflicts with M_E, M_LOG2, ...
#include <BRepPrimAPI_MakeBox.hxx>

#include "test.h"
#include "../src/base/brep_utils.h"
#include "../src/base/libtree.h"
#include "../src/base/geom_utils.h"
#include "../src/base/mesh_utils.h"
#include "../src/base/result.h"
#include "../src/base/string_utils.h"
#include "../src/base/unit.h"
#include "../src/base/unit_system.h"

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <QtCore/QFile>
#include <QtCore/QtDebug>
#include <cmath>
#include <cstring>
#include <utility>
#include <iostream>
#include <sstream>

Q_DECLARE_METATYPE(Mayo::UnitSystem::TranslateResult)
// For MeshUtils_orientation_test()
Q_DECLARE_METATYPE(std::vector<gp_Pnt2d>)
Q_DECLARE_METATYPE(Mayo::MeshUtils::Orientation)

namespace Mayo {

// For the sake of QCOMPARE()
static bool operator==(
        const UnitSystem::TranslateResult& lhs,
        const UnitSystem::TranslateResult& rhs)
{
    return std::abs(lhs.value - rhs.value) < 1e-6
            && std::strcmp(lhs.strUnit, rhs.strUnit) == 0
            && std::abs(lhs.factor - rhs.factor) < 1e-6;
}

void Test::BRepUtils_test()
{
    QVERIFY(BRepUtils::moreComplex(TopAbs_COMPOUND, TopAbs_SOLID));
    QVERIFY(BRepUtils::moreComplex(TopAbs_SOLID, TopAbs_SHELL));
    QVERIFY(BRepUtils::moreComplex(TopAbs_SHELL, TopAbs_FACE));
    QVERIFY(BRepUtils::moreComplex(TopAbs_FACE, TopAbs_EDGE));
    QVERIFY(BRepUtils::moreComplex(TopAbs_EDGE, TopAbs_VERTEX));

    {
        const TopoDS_Shape shapeNull;
        const TopoDS_Shape shapeBase = BRepPrimAPI_MakeBox(25, 25, 25);
        const TopoDS_Shape shapeCopy = shapeBase;
        QCOMPARE(BRepUtils::hashCode(shapeNull), -1);
        QVERIFY(BRepUtils::hashCode(shapeBase) >= 0);
        QCOMPARE(BRepUtils::hashCode(shapeBase), BRepUtils::hashCode(shapeCopy));
    }
}

void Test::CafUtils_test()
{
    // TODO Add CafUtils::labelTag() test for multi-threaded safety
}

void Test::MeshUtils_orientation_test()
{
    struct BasicPolyline2d : public Mayo::MeshUtils::AdaptorPolyline2d {
        gp_Pnt2d pointAt(int index) const override { return this->vecPoint.at(index); }
        int pointCount() const override { return this->vecPoint.size(); }
        std::vector<gp_Pnt2d> vecPoint;
    };

    QFETCH(std::vector<gp_Pnt2d>, vecPoint);
    QFETCH(Mayo::MeshUtils::Orientation, orientation);
    BasicPolyline2d polyline2d;
    polyline2d.vecPoint = std::move(vecPoint);
    QCOMPARE(Mayo::MeshUtils::orientation(polyline2d), orientation);
}

void Test::MeshUtils_orientation_test_data()
{
    QTest::addColumn<std::vector<gp_Pnt2d>>("vecPoint");
    QTest::addColumn<Mayo::MeshUtils::Orientation>("orientation");

    std::vector<gp_Pnt2d> vecPoint;
    vecPoint.push_back(gp_Pnt2d(0, 0));
    vecPoint.push_back(gp_Pnt2d(0, 10));
    vecPoint.push_back(gp_Pnt2d(10, 10));
    vecPoint.push_back(gp_Pnt2d(10, 0));
    vecPoint.push_back(gp_Pnt2d(0, 0)); // Closed polyline
    QTest::newRow("case1") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    vecPoint.erase(std::prev(vecPoint.end())); // Open polyline
    QTest::newRow("case2") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    std::reverse(vecPoint.begin(), vecPoint.end());
    QTest::newRow("case3") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

    vecPoint.clear();
    vecPoint.push_back(gp_Pnt2d(0, 0));
    vecPoint.push_back(gp_Pnt2d(0, 10));
    vecPoint.push_back(gp_Pnt2d(10, 10));
    QTest::newRow("case4") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    vecPoint.clear();
    vecPoint.push_back(gp_Pnt2d(0, 0));
    vecPoint.push_back(gp_Pnt2d(0, 10));
    vecPoint.push_back(gp_Pnt2d(-10, 10));
    vecPoint.push_back(gp_Pnt2d(-10, 0));
    QTest::newRow("case5") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

    std::reverse(vecPoint.begin(), vecPoint.end());
    QTest::newRow("case6") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    {
        QFile file("inputs/mayo_bezier_curve.brep");
        QVERIFY(file.open(QIODevice::ReadOnly));

        const TopoDS_Shape shape = BRepUtils::shapeFromString(file.readAll().toStdString());
        QVERIFY(!shape.IsNull());

        TopoDS_Edge edge;
        BRepUtils::forEachSubShape(shape, TopAbs_EDGE, [&](const TopoDS_Shape& subShape) {
            edge = TopoDS::Edge(subShape);
        });
        QVERIFY(!edge.IsNull());

        const GCPnts_TangentialDeflection discr(BRepAdaptor_Curve(edge), 0.1, 0.1);
        vecPoint.clear();
        for (int i = 1; i <= discr.NbPoints(); ++i) {
            const gp_Pnt pnt = discr.Value(i);
            vecPoint.push_back(gp_Pnt2d(pnt.X(), pnt.Y()));
        }

        QTest::newRow("case7") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

        std::reverse(vecPoint.begin(), vecPoint.end());
        QTest::newRow("case8") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;
    }
}

void Test::MeshUtils_test()
{
    // Create box
    QFETCH(double, boxDx);
    QFETCH(double, boxDy);
    QFETCH(double, boxDz);
    const TopoDS_Shape shapeBox = BRepPrimAPI_MakeBox(boxDx, boxDy, boxDz);

    // Mesh box
    {
        BRepMesh_IncrementalMesh mesher(shapeBox, 0.1);
        mesher.Perform();
        QVERIFY(mesher.IsDone());
    }

    // Count nodes and triangles
    int countNode = 0;
    int countTriangle = 0;
    BRepUtils::forEachSubFace(shapeBox, [&](const TopoDS_Face& face) {
        TopLoc_Location loc;
        const Handle_Poly_Triangulation& polyTri = BRep_Tool::Triangulation(face, loc);
        if (!polyTri.IsNull()) {
            countNode += polyTri->NbNodes();
            countTriangle += polyTri->NbTriangles();
        }
    });

    // Merge all face triangulations into one
    Handle_Poly_Triangulation polyTriBox =
            new Poly_Triangulation(countNode, countTriangle, false);
    {
        int idNodeOffset = 0;
        int idTriangleOffset = 0;
        BRepUtils::forEachSubFace(shapeBox, [&](const TopoDS_Face& face) {
            TopLoc_Location loc;
            const Handle_Poly_Triangulation& polyTri = BRep_Tool::Triangulation(face, loc);
            if (!polyTri.IsNull()) {
                for (int i = 1; i <= polyTri->NbNodes(); ++i)
                    polyTriBox->ChangeNode(idNodeOffset + i) = polyTri->Node(i);

                for (int i = 1; i <= polyTri->NbTriangles(); ++i) {
                    int n1, n2, n3;
                    polyTri->Triangle(i).Get(n1, n2, n3);
                    polyTriBox->ChangeTriangle(idTriangleOffset + i).Set(
                                idNodeOffset + n1, idNodeOffset + n2, idNodeOffset + n3);
                }

                idNodeOffset += polyTri->NbNodes();
                idTriangleOffset += polyTri->NbTriangles();
            }
        });
    }

    // Checks
    QCOMPARE(MeshUtils::triangulationVolume(polyTriBox),
             double(boxDx * boxDy * boxDz));
    QCOMPARE(MeshUtils::triangulationArea(polyTriBox),
             double(2 * boxDx * boxDy + 2 * boxDy * boxDz + 2 * boxDx * boxDz));
}

void Test::MeshUtils_test_data()
{
    QTest::addColumn<double>("boxDx");
    QTest::addColumn<double>("boxDy");
    QTest::addColumn<double>("boxDz");

    QTest::newRow("case1") << 10. << 15. << 20.;
    QTest::newRow("case2") << 0.1 << 0.25 << 0.044;
    QTest::newRow("case3") << 1e5 << 1e6 << 1e7;
    QTest::newRow("case4") << 40. << 50. << 70.;
}

void Test::Quantity_test()
{
    const QuantityArea area = (10 * Quantity_Millimeter) * (5 * Quantity_Centimeter);
    QCOMPARE(area.value(), 500.);
    QCOMPARE((Quantity_Millimeter / 5.).value(), 1/5.);
}

namespace Result_test {

struct Data {
    static std::ostream* data_ostr;

    Data() {
        *data_ostr << 0;
    }
    Data(const Data& other) : foo(other.foo) {
        *data_ostr << 1;
    }
    Data(Data&& other) {
        foo = std::move(other.foo);
        *data_ostr << 2;
    }
    Data& operator=(const Data& other) {
        this->foo = other.foo;
        *data_ostr << 3;
        return *this;
    }
    Data& operator=(Data&& other) {
        this->foo = std::move(other.foo);
        *data_ostr << 4;
        return *this;
    }
    QString foo;
};
std::ostream* Data::data_ostr = nullptr;

} // Result_test

void Test::Result_test()
{
    using Result = Result<Result_test::Data>;
    {
        std::ostringstream sstr;
        Result::Data::data_ostr = &sstr;
        const Result res = Result::error("error_description");
        QVERIFY(res.errorText() == "error_description");
        QVERIFY(!res.valid());
        QCOMPARE(sstr.str().c_str(), "02");
    }
    {
        std::ostringstream sstr;
        Result::Data::data_ostr = &sstr;
        Result::Data data;
        data.foo = "FooData";
        const Result res = Result::ok(std::move(data));
        QVERIFY(res.valid());
        QVERIFY(res.get().foo == "FooData");
        QCOMPARE(sstr.str().c_str(), "0042");
    }
}

void Test::StringUtils_append_test()
{
    QFETCH(QString, strExpected);
    QFETCH(QString, str1);
    QFETCH(QString, str2);
    QFETCH(QLocale, locale);

    QString strActual = str1;
    StringUtils::append(&strActual, str2, locale);
    QCOMPARE(strActual, strExpected);
}

void Test::StringUtils_append_test_data()
{
    QTest::addColumn<QString>("strExpected");
    QTest::addColumn<QString>("str1");
    QTest::addColumn<QString>("str2");
    QTest::addColumn<QLocale>("locale");

    QTest::newRow("locale_c")
            << QString("1234")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale::c();
    QTest::newRow("locale_fr")
            << QString("1234")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale(QLocale::French);
    QTest::newRow("locale_arabic")
            << QString("3412")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale(QLocale::Arabic);
    QTest::newRow("locale_syriac")
            << QString("3412")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale(QLocale::Syriac);
}

void Test::StringUtils_text_test()
{
    QFETCH(QString, strActual);
    QFETCH(QString, strExpected);
    QCOMPARE(strActual, strExpected);
}

void Test::StringUtils_text_test_data()
{
    QTest::addColumn<QString>("strActual");
    QTest::addColumn<QString>("strExpected");

    const StringUtils::TextOptions opts_c_si_2 = { QLocale::c(), UnitSystem::SI, 2 };
    const StringUtils::TextOptions opts_fr_si_2 = {
        QLocale(QLocale::French, QLocale::France), UnitSystem::SI, 2 };
    QTest::newRow("c_0.1")
            << StringUtils::text(0.1, opts_c_si_2)
            << QStringLiteral("0.1");
    QTest::newRow("c_0.155")
            << StringUtils::text(0.155, opts_c_si_2)
            << QStringLiteral("0.15");
    QTest::newRow("c_0.159")
            << StringUtils::text(0.159, opts_c_si_2)
            << QStringLiteral("0.16");
    QTest::newRow("fr_1.4995")
            << StringUtils::text(1.4995, opts_fr_si_2)
            << QStringLiteral("1,5");
    QTest::newRow("c_pnt0.55,4.8977,15.1445")
            << StringUtils::text(gp_Pnt(0.55, 4.8977, 15.1445), opts_c_si_2)
            << QStringLiteral("(0.55mm 4.9mm 15.14mm)");
}

void Test::StringUtils_skipWhiteSpaces_test()
{
    QFETCH(ptrdiff_t, ptrDiffActualExpected);
    QCOMPARE(ptrDiffActualExpected, ptrdiff_t(0));
}

void Test::StringUtils_skipWhiteSpaces_test_data()
{
    QTest::addColumn<ptrdiff_t>("ptrDiffActualExpected");

    const char str1[] = "test";
    QTest::newRow("skipws_ident")
            << StringUtils::skipWhiteSpaces(str1, sizeof(str1)) - str1;
    const char str2[] = "    test";
    QTest::newRow("skipws_4spaces")
            << StringUtils::skipWhiteSpaces(str2, sizeof(str2)) - (str2 + 4);
    const char str3[] = " \n\t test";
    QTest::newRow("skipws_4mixed")
            << StringUtils::skipWhiteSpaces(str3, sizeof(str3)) - (str3 + 4);
    const char str4[] = "";
    QTest::newRow("skipws_empty")
            << StringUtils::skipWhiteSpaces(str4, sizeof(str4)) - str4;
}

void Test::UnitSystem_test()
{
    QFETCH(UnitSystem::TranslateResult, trResultActual);
    QFETCH(UnitSystem::TranslateResult, trResultExpected);
    QCOMPARE(trResultActual, trResultExpected);
}

void Test::UnitSystem_test_data()
{
    QTest::addColumn<UnitSystem::TranslateResult>("trResultActual");
    QTest::addColumn<UnitSystem::TranslateResult>("trResultExpected");

    const UnitSystem::Schema schemaSI = UnitSystem::SI;
    QTest::newRow("80mm")
            << UnitSystem::translate(schemaSI, 80 * Quantity_Millimeter)
            << UnitSystem::TranslateResult{ 80., "mm", 1. };
    QTest::newRow("8cm")
            << UnitSystem::translate(schemaSI, 8 * Quantity_Centimeter)
            << UnitSystem::TranslateResult{ 80., "mm", 1. };
    QTest::newRow("8m")
            << UnitSystem::translate(schemaSI, 8 * Quantity_Meter)
            << UnitSystem::TranslateResult{ 8000., "mm", 1. };
    QTest::newRow("50mm²")
            << UnitSystem::translate(schemaSI, 0.5 * Quantity_SquaredCentimer)
            << UnitSystem::TranslateResult{ 50., "mm²", 1. };
    constexpr double radDeg = Quantity_Degree.value();
    QTest::newRow("degrees(PIrad)")
            << UnitSystem::degrees(3.14159265358979323846 * Quantity_Radian)
            << UnitSystem::TranslateResult{ 180., "°", radDeg };
}

void Test::LibTree_test()
{
    const TreeNodeId nullptrId = 0;
    Tree<std::string> tree;
    TreeNodeId n0 = tree.appendChild(0, "0");
    TreeNodeId n0_1 = tree.appendChild(n0, "0-1");
    TreeNodeId n0_2 = tree.appendChild(n0, "0-2");
    TreeNodeId n0_1_1 = tree.appendChild(n0_1, "0-1-1");
    TreeNodeId n0_1_2 = tree.appendChild(n0_1, "0-1-2");

    QCOMPARE(tree.nodeParent(n0_1), n0);
    QCOMPARE(tree.nodeParent(n0_2), n0);
    QCOMPARE(tree.nodeParent(n0_1_1), n0_1);
    QCOMPARE(tree.nodeParent(n0_1_2), n0_1);
    QCOMPARE(tree.nodeChildFirst(n0_1), n0_1_1);
    QCOMPARE(tree.nodeChildLast(n0_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingNext(n0_1_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingPrevious(n0_1_2), n0_1_1);
    QCOMPARE(tree.nodeSiblingNext(n0_1_2), nullptrId);
}

} // namespace Mayo

