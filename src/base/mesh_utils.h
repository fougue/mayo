/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_handle.h"

#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Version.hxx>
class gp_XYZ;

namespace Mayo {

// Provides helper functions for mesh and triangle objects
namespace MeshUtils {

double triangleSignedVolume(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);
double triangleArea(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);

double triangulationVolume(const OccHandle<Poly_Triangulation>& triangulation);
double triangulationArea(const OccHandle<Poly_Triangulation>& triangulation);

#if OCC_VERSION_HEX >= 0x070600
using Poly_Triangulation_NormalType = gp_Vec3f;
#else
using Poly_Triangulation_NormalType = gp_Vec;
#endif

void setNode(const OccHandle<Poly_Triangulation>& triangulation, int index, const gp_Pnt& pnt);
void setNormal(const OccHandle<Poly_Triangulation>& triangulation, int index, const Poly_Triangulation_NormalType& n);
void setTriangle(const OccHandle<Poly_Triangulation>& triangulation, int index, const Poly_Triangle& triangle);
void setUvNode(const OccHandle<Poly_Triangulation>& triangulation, int index, double u, double v);

void allocateNormals(const OccHandle<Poly_Triangulation>& triangulation);

Poly_Triangulation_NormalType normal(const OccHandle<Poly_Triangulation>& triangulation, int index);
const Poly_Array1OfTriangle& triangles(const OccHandle<Poly_Triangulation>& triangulation);

enum class Orientation {
    Unknown,
    Clockwise,
    CounterClockwise
};

class AdaptorPolyline2d {
public:
    virtual gp_Pnt2d pointAt(int index) const = 0;
    virtual int pointCount() const = 0;
    virtual bool empty() const { return this->pointCount() <= 0; }
};

class AdaptorPolyline3d {
public:
    virtual const gp_Pnt& pointAt(int i) const = 0;
    virtual int pointCount() const = 0;
    virtual int empty() const { return this->pointCount() <= 0; }
};

Orientation orientation(const AdaptorPolyline2d& polyline);
gp_Vec directionAt(const AdaptorPolyline3d& polyline, int i);

// Provides helper to create Poly_Polygon3D objects
// Poly_Polygon3D class interface changed from OpenCascade 7.4 to 7.5 version so using this class
// directly might cause compilation errors
// Prefer Polygon3dBuilder so application code doesn't have to care about OpenCascade version
class Polygon3dBuilder {
public:
    enum class ParametersOption { None, With };

    Polygon3dBuilder(int nodeCount, ParametersOption option = ParametersOption::None);

    void setNode(int i, const gp_Pnt& pnt);
    void setParameter(int i, double u);
    void finalize();
    OccHandle<Poly_Polygon3D> get() const;

private:
    bool m_isFinalized = false;
    OccHandle<Poly_Polygon3D> m_polygon;
#if OCC_VERSION_HEX < 0x070500
    TColgp_Array1OfPnt m_nodes;
    TColStd_Array1OfReal m_params;
#endif
    TColgp_Array1OfPnt* m_ptrNodes = nullptr;
    TColStd_Array1OfReal* m_ptrParams = nullptr;
};

} // namespace MeshUtils
} // namespace Mayo
