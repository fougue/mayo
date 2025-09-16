/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QVariant>
#include <QtQml/QJSValue>

namespace Mayo {

class ScriptApplication;
class ScriptDocument;

#ifndef _MAYO_DOCGEN_

// Mayo, Application, Document
using QJSValue_MayoTraverseModelTreeCallback = QJSValue;
using QJSValue_MayoTraverseShapeCallback = QJSValue;
using QJSValue_ScriptDocument = QJSValue;
using QJSValue_JsonObject = QJSValue;
using QJSValueList_ArrayOfTaskId = QJSValueList;

using Ptr_ScriptApplication = ScriptApplication*;
using Ptr_ScriptDocument = ScriptDocument*;

// Geom
using QVariant_Coords3D = QVariant;
using QVariant_ScriptGeomAx1 = QVariant;
using QVariant_ScriptGeomAx3 = QVariant;

using ScriptGeomContinuity = unsigned; // ->GeomAbs_Shape
using ScriptGeomBSplineKnotDistribution = unsigned; // ->GeomAbs_BSplKnotDistribution

// GeomCurve
using QVariant_ScriptGeomCurve = QVariant;
using QVariant_ScriptGeomLine = QVariant;
using QVariant_ScriptGeomCircle = QVariant;
using QVariant_ScriptGeomEllipse = QVariant;
using QVariant_ScriptGeomHyperbola = QVariant;
using QVariant_ScriptGeomParabola = QVariant;
using QVariant_ScriptGeomBezierCurve = QVariant;
using QVariant_ScriptGeomBSplineCurve = QVariant;
using QVariant_ScriptGeomOffsetCurve = QVariant;

using ScriptGeomCurveType = unsigned;  // ->GeomAbs_CurveType

// GeomSurface
using QVariant_ScriptGeomSurface = QVariant;
using QVariant_ScriptGeomCylinder = QVariant;
using QVariant_ScriptGeomPlane = QVariant;
using QVariant_ScriptGeomCone = QVariant;
using QVariant_ScriptGeomSphere = QVariant;
using QVariant_ScriptGeomTorus = QVariant;
using QVariant_ScriptGeomBezierSurface = QVariant;
using QVariant_ScriptGeomBSplineSurface = QVariant;
using QVariant_ScriptGeomSurfaceOfLinearExtrusion = QVariant;
using QVariant_ScriptGeomSurfaceOfRevolution = QVariant;
using QVariant_ScriptGeomOffsetSurface = QVariant;

using ScriptGeomSurfaceType = unsigned;  // ->GeomAbs_SurfaceType

// Shape
using QVariant_ScriptShape = QVariant;
using QJSValue_ScriptShape = QJSValue;
using ScriptShapeType = unsigned; // ->TopAbs_ShapeEnum
using ScriptShapeOrientation = unsigned; // ->TopAbs_Orientation

// TreeNode
using QVariant_ScriptTreeNode = QVariant;

#endif

} // namespace Mayo
