/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/brep_utils.h"
#include "../src/base/mesh_utils.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <vector>

Q_DECLARE_METATYPE(std::vector<gp_Pnt2d>)
Q_DECLARE_METATYPE(Mayo::MeshUtils::Orientation)

namespace Mayo {

void TestBase::MeshUtils_orientation_test()
{
    struct BasicPolyline2d : public Mayo::MeshUtils::AdaptorPolyline2d {
        gp_Pnt2d pointAt(int index) const override { return this->vecPoint.at(index); }
        int pointCount() const override { return int(this->vecPoint.size()); }
        std::vector<gp_Pnt2d> vecPoint;
    };

    QFETCH(std::vector<gp_Pnt2d>, vecPoint);
    QFETCH(Mayo::MeshUtils::Orientation, orientation);
    BasicPolyline2d polyline2d;
    polyline2d.vecPoint = std::move(vecPoint);
    QCOMPARE(Mayo::MeshUtils::orientation(polyline2d), orientation);
}

void TestBase::MeshUtils_orientation_test_data()
{
    QTest::addColumn<std::vector<gp_Pnt2d>>("vecPoint");
    QTest::addColumn<Mayo::MeshUtils::Orientation>("orientation");

    {
        // Closed polyline
        const std::vector<gp_Pnt2d> vecPoint = {{0,0}, {0,10}, {10,10}, {10,0}, {0,0}};
        QTest::newRow("case1") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;
    }

    {
        // Open polyline
        const std::vector<gp_Pnt2d> vecPoint = {{0,0}, {0,10}, {10,10}, {10,0}};
        QTest::newRow("case2") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;
    }

    {
        const std::vector<gp_Pnt2d> vecPoint = {{10,0}, {10,10}, {0,10}, {0,0}};
        QTest::newRow("case3") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;
    }

    {
        const std::vector<gp_Pnt2d> vecPoint = {{0,0}, {0,10}, {10,10}};
        QTest::newRow("case4") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;
    }

    {
        const std::vector<gp_Pnt2d> vecPoint = {{0,0}, {0,10}, {-10,10}, {-10,0}};
        QTest::newRow("case5") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;
    }

    {
        const std::vector<gp_Pnt2d> vecPoint = {{-10,0}, {-10,10}, {0,10}, {0,0}};
        QTest::newRow("case6") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;
    }

    {
        std::vector<gp_Pnt2d> vecPoint = {
            {-2.07,-0.81}, {-1.98,-0.68}, {-1.87,-0.59}, {-1.76,-0.51}, {-1.65,-0.46}, {-1.52,-0.43},
            {-1.36,-0.41}, {-1.09,-0.40}, {-0.10,-0.44}, {0.38,-0.38}, {0.70,-0.27}, {0.92,-0.14},
            {1.08,-0.01}, {1.16,0.09}, {1.19,0.16}, {1.21,0.21}, {1.21,0.25}, {1.21,0.27}, {1.20,0.30},
            {1.19,0.31}, {1.18,0.33}, {1.17,0.34}, {1.15,0.35}, {1.13,0.36}, {1.11,0.37}, {1.08,0.37},
            {1.05,0.37}, {1.00,0.37}, {0.95,0.35}, {0.87,0.33}, {0.77,0.27}, {0.53,0.11}, {0.48,0.09},
            {0.45,0.09}, {0.43,0.09}, {0.41,0.09}, {0.39,0.10}, {0.37,0.11}, {0.35,0.12}, {0.34,0.14},
            {0.32,0.16}, {0.31,0.19}, {0.29,0.24}, {0.28,0.30}, {0.24,0.55}, {0.21,0.62}, {0.18,0.67},
            {0.13,0.73}, {0.07,0.78}, {-0.01,0.83}, {-0.12,0.88}, {-0.24,0.91}, {-0.44,0.95}, {-0.70,0.96},
            {-0.98,0.93}, {-1.27,0.87}, {-1.54,0.76}, {-1.72,0.65}, {-1.85,0.53}, {-1.95,0.40}, {-2.02,0.27},
            {-2.06,0.12}, {-2.09,-0.05}, {-2.10,-0.40}, {-2.07,-0.81}
        };
        QTest::newRow("case7") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

        std::vector<gp_Pnt2d> vecPointReversed = vecPoint;
        std::reverse(vecPointReversed.begin(), vecPointReversed.end());
        QTest::newRow("case8") << vecPointReversed << Mayo::MeshUtils::Orientation::Clockwise;
    }
}

void TestBase::MeshUtils_test()
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
        const OccHandle<Poly_Triangulation>& polyTri = BRep_Tool::Triangulation(face, loc);
        if (!polyTri.IsNull()) {
            countNode += polyTri->NbNodes();
            countTriangle += polyTri->NbTriangles();
        }
    });

    // Merge all face triangulations into one
    auto polyTriBox = makeOccHandle<Poly_Triangulation>(countNode, countTriangle, false);
    {
        int idNodeOffset = 0;
        int idTriangleOffset = 0;
        BRepUtils::forEachSubFace(shapeBox, [&](const TopoDS_Face& face) {
            TopLoc_Location loc;
            const OccHandle<Poly_Triangulation>& polyTri = BRep_Tool::Triangulation(face, loc);
            if (!polyTri.IsNull()) {
                for (int i = 1; i <= polyTri->NbNodes(); ++i)
                    MeshUtils::setNode(polyTriBox, idNodeOffset + i, polyTri->Node(i));

                for (int i = 1; i <= polyTri->NbTriangles(); ++i) {
                    int n1, n2, n3;
                    polyTri->Triangle(i).Get(n1, n2, n3);
                    MeshUtils::setTriangle(
                        polyTriBox,
                        idTriangleOffset + i,
                        { idNodeOffset + n1, idNodeOffset + n2, idNodeOffset + n3 }
                    );
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

void TestBase::MeshUtils_test_data()
{
    QTest::addColumn<double>("boxDx");
    QTest::addColumn<double>("boxDy");
    QTest::addColumn<double>("boxDz");

    QTest::newRow("case1") << 10. << 15. << 20.;
    QTest::newRow("case2") << 0.1 << 0.25 << 0.044;
    QTest::newRow("case3") << 1e5 << 1e6 << 1e7;
    QTest::newRow("case4") << 40. << 50. << 70.;
}

} // namespace Mayo
