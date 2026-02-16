/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <vector>

#include "dxf_format_common.h"

// NOTE
//   According AUTOCAD documentation "Group Codes in Numerical Order" section states that codes 50-58
//   are expressed in degrees

// Common group codes for entities
struct Dxf_BaseEntity {
    // Code 5
    DxfStringRef handle;
    // Code 6
    DxfStringRef lineTypeName;
    // Code 8
    DxfStringRef layerName;
    // Code 60
    bool isVisible = true;
    // Code 62
    DxfColorIndex colorId = dxfColorByLayer;
    // Code 67
    enum class Space { Model = 0, Paper = 1 };
    Space space = Space::Model;
};

struct Dxf_BaseGeom2dEntity : public Dxf_BaseEntity {
    // Code 39
    double thickness = 0.;
    // Code 210, 220, 230
    DxfCoords extrusionDirection = {0., 0., 1.};
};

struct Dxf_POINT : public Dxf_BaseGeom2dEntity {
    // Code 10, 20, 30
    DxfCoords location;
    // Code 50
    double angleXAxis = 0.; // degrees
};

struct Dxf_TEXT : public Dxf_BaseGeom2dEntity {
    // Code 1
    DxfStringRef str;
    // Code 7  text style name(default=STANDARD)
    DxfStringRef styleName;
    // Code 10, 20, 30
    DxfCoords firstAlignmentPoint = {};  // OCS
    // Code 11, 21, 31
    //   AUTOCAD doc: "This value is meaningful only if the value of a 72 or 73 group is nonzero (if
    //   the justification is anything other than baseline/left)"
    DxfCoords secondAlignmentPoint = {}; // OCS
    // Code 40
    double height = 0.;
    // Code 41  (AUTOCAD doc: "This value is also adjusted when fit-type text is used")
    double relativeXScaleFactorWidth = 1.;
    // Code 50
    double rotationAngle = 0.; // degrees
    // Code 51
    double obliqueAngle = 0.;  // degrees
    // Code 71  Text generation flags
    //   0: default
    //   2: text is backward(mirrored in X)
    //   4: text is upside down(mirrored in Y)
    unsigned generationFlags = 0;
    // Code 72
    enum class HorizontalJustification {
        Left = 0, Center = 1, Right = 2,
        Aligned = 3, // If vertical alignment == 0
        Middle = 4,  // If vertical alignment == 0
        Fit = 5      // If vertical alignment == 0
    };
    HorizontalJustification horizontalJustification = HorizontalJustification::Left;
    // Code 73
    enum class VerticalJustification {
        Baseline = 0, Bottom = 1, Middle = 2, Top = 3
    };
    VerticalJustification verticalJustification = VerticalJustification::Baseline;
};

struct Dxf_ATTRIB : public Dxf_TEXT {
    // Code 2  (AUTOCAD doc: "cannot contain spaces")
    DxfStringRef tag;
    // Code 70
    //   1: attribute is invisible(does not appear)
    //   2: this is a constant attribute
    //   4: verification is required on input of this attribute
    //   8: attribute is preset(no prompt during insertion)
    unsigned flags = 0;
    // Code 73  (AUTOCAD doc: "not currently used")
    double fixedLength = 0.;
    // Code 340
    DxfStringRef mtextHandle;

    // NOTE
    //   Code 10, 20, 30 "textStartPoint" -> same meaning as for Dxf_TEXT::firstAlignmentPoint
    //   Code 11, 21, 31 "alignmentPoint" -> same meaning as for Dxf_TEXT::secondAlignmentPoint
    //   AUTOCAD doc: "Present only if 72 or 74 group is present and nonzero"

    // NOTE
    //   Code 74 is the same as TEXT code 73
};

struct Dxf_MTEXT : public Dxf_BaseGeom2dEntity {
    // Code 1, 3
    DxfStringRef str;
    // Code 7  text style name(default=STANDARD)
    DxfStringRef styleName;
    // Code 10, 20, 30
    DxfCoords insertionPoint = {};
    // Code 11, 21, 31
    DxfCoords xAxisDirection = {1., 0., 0.}; // WCS
    // Code 40
    double height = 0.;
    // Code 41
    double referenceRectangleWidth = 0;
    // Code 44
    // Percentage of default(3-on-5) line spacing to be applied. Valid values range from 0.25 to 4.0
    double lineSpacingFactor = 1.;
    // Code 50
    double rotationAngle = 0.; // degrees
    // Code 71
    enum class AttachmentPoint {
        TopLeft = 1, TopCenter,    TopRight,
        MiddleLeft,  MiddleCenter, MiddleRight,
        BottomLeft,  BottomCenter, BottomRight
    };
    AttachmentPoint attachmentPoint = AttachmentPoint::TopLeft;
    // Code 72(drawing direction)
    //     1: Left to right
    //     3: Top to bottom
    //     5: By style(the flow direction is inherited from the associated text style)
    unsigned drawingDirection = 1;
    // Code 73
    //     1: At least(taller characters will override)
    //     2: Exact(taller characters will not override)
    unsigned lineSpacingStyle = 0;

    // NOTE AUTOCAD documentation states that codes 42, 43 are "read-only, ignored if supplied"

    // TODO Code 90(background fill setting)
    // TODO Code 420-429(background color, if RGB)
    // TODO Code 430-439(background color, if name)
    // TODO Code 45(fill box scale)
    // TODO Code 63(background fill color)
    // TODO Code 441(transparency of background fill color)
    // TODO Codes for columns: 75, 76, 78, 79, 48, 49, 50

    enum class ColumnType { None = 0, Static, Dynamic };
    bool acadHasColumnInfo = false;
    ColumnType acadColumnInfo_Type = ColumnType::None;
    bool acadColumnInfo_AutoHeight = false;
    int acadColumnInfo_Count = 0;
    bool acadColumnInfo_FlowReversed = false;
    double acadColumnInfo_Width = 0.;
    double acadColumnInfo_GutterWidth = 0.;

    bool acadHasDefinedHeight = false;
    double acadDefinedHeight = 0.;
};

struct Dxf_LINE : public Dxf_BaseGeom2dEntity {
    // Code 10, 20, 30
    DxfCoords startPoint = {};
    // Code 11, 21, 31
    DxfCoords endPoint = {};
};

struct Dxf_POLYLINE : public Dxf_BaseGeom2dEntity {
    struct Vertex {
        enum Flag {
            None = 0,
            ExtraVertex = 1,
            CurveFitTangent = 2,
            NotUsed = 4,
            SplineVertex = 8,
            SplineFrameControlPoint = 16,
            Polyline3dVertex = 32,
            Polygon3dVertex = 64,
            PolyfaceMeshVertex = 128
        };
        using Flags = unsigned;

        // Code 10, 20, 30
        DxfCoords point = {};
        // Code 40
        double startingWidth = 0.;
        // Code 41
        double endingWidth = 0.;
        // Code 42
        double bulge = 0.;
        // Code 50
        // If flags & CurveFitTangent, should appear only on first and last vertices
        double curveFitTangentDirection = 0.;
        // Code 70
        Flags flags = Flag::None;
        // Code 71
        int polyfaceMeshVertex1 = 0;
        // Code 72
        int polyfaceMeshVertex2 = 0;
        // Code 73
        int polyfaceMeshVertex3 = 0;
        // Code 74
        int polyfaceMeshVertex4 = 0;
        // Code 91
        // int identifier = 0;
    };

    enum Flag {
        None = 0,
        Closed = 1,
        CurveFit = 2,
        SplineFit = 4,
        Polyline3d = 8,
        PolygonMesh3d = 16,
        PolygonMeshClosedNDir = 32,
        PolyfaceMesh = 64,
        ContinuousLinetypePattern = 128
    };
    using Flags = unsigned;

    enum PolylineType {
        NoSmoothSurfaceFitted = 0,
        QuadraticBSplineSurface = 5,
        CubicBSplineSurface = 6,
        BezierSurface = 8
    };

    // Code 70
    Flags flags = Flag::None;
    // Code 40
    double defaultStartWidth = 0.;
    // Code 41
    double defaultEndWidth = 0.;
    // Code 71(number of vertices in the mesh)
    int polygonMeshMVertexCount = 0;
    // Code 72(number of faces in the mesh)
    int polygonMeshNVertexCount = 0;
    // Code 73
    double smoothSurfaceMDensity = 0.;
    // Code 74
    double smoothSurfaceNDensity = 0.;
    // Code 75
    PolylineType polylineType = PolylineType::NoSmoothSurfaceFitted;

    std::vector<Vertex> vertices;
};

struct Dxf_LWPOLYLINE : public Dxf_BaseGeom2dEntity {
    struct Vertex {
        // Code 10
        double x = 0.;
        // Code 20
        double y = 0.;
        // Code 40
        double startWidth = 0.;
        // Code 41
        double endWidth = 0.;
        // Code 42
        double bulge = 0.;
    };

    // Code 38
    double elevation = 0.;
    // Code 43
    double constantWidth = 0.;
    // Code 70 (1: Closed, 128: Plinegen)
    unsigned flag = 0;

    std::vector<Vertex> vertices;
};

struct Dxf_INSERT : public Dxf_BaseEntity {
    // Code 2
    DxfStringRef blockName;
    // Code 10, 20, 30
    DxfCoords insertPoint = {}; // OCS
    // Code 41, 42, 43
    DxfScale scaleFactor = { 1., 1., 1. };
    // Code 50
    double rotationAngle = 0.; // degrees
    // Code 70
    int columnCount = 1;
    // Code 71
    int rowCount = 1;
    // Code 44
    double columnSpacing = 0.;
    // Code 45
    double rowSpacing = 0.;
    // Code 210, 220, 230
    DxfCoords extrusionDirection = { 0., 0., 1. };
};

struct Dxf_QuadBase {
    // Code 10, 20, 30
    DxfCoords corner1;
    // Code 11, 21, 31
    DxfCoords corner2;
    // Code 12, 22, 32
    DxfCoords corner3;
    // Code 13, 23, 33
    DxfCoords corner4;
    bool hasCorner4 = false;
};

struct Dxf_3DFACE : public Dxf_BaseEntity, public Dxf_QuadBase {
    enum Flag {
        None = 0,
        InvisibleEdge1 = 1,
        InvisibleEdge2 = 2,
        InvisibleEdge3 = 4,
        InvisibleEdge4 = 8
    };
    using Flags = unsigned;

    // Code 70
    Flags flags = Flag::None;
};

struct Dxf_SOLID : public Dxf_BaseGeom2dEntity, public Dxf_QuadBase {
};

struct Dxf_SPLINE : public Dxf_BaseEntity {
    enum Flag {
        None = 0,
        Closed = 1,
        Periodic = 2,
        Rational = 4,
        Planar = 8,
        Linear = 16 // Planar bit is also set
    };
    using Flags = unsigned;

    // Code 210, 220, 230
    DxfCoords normalVector = { 0., 0., 1. };
    // Code 70
    Flags flags = Flag::None;
    // Code 71
    int degree = 0;

    // NOTE: value of code 72 is knots.size()
    // NOTE: value of code 73 is controlPoints.size()
    // NOTE: value of code 74 is fitPoints.size()

    // Code 42
    double knotTolerance = 0.0000001;
    // Code 43
    double controlPointTolerance = 0.0000001;
    // Code 44
    double fitTolerance = 0.0000000001;

    // Code 12, 22, 32
    DxfCoords startTangent = {};
    // Code 13, 23, 33
    DxfCoords endTangent = {};
    // Code 40
    std::vector<double> knots;
    // Code 41
    std::vector<double> weights;
    // Code 10, 20, 30
    std::vector<DxfCoords> controlPoints;
    // Code 11, 21, 31
    std::vector<DxfCoords> fitPoints;
};

struct Dxf_ARC : public Dxf_BaseGeom2dEntity {
    // Code 10, 20, 30
    DxfCoords centerPoint = {};
    // Code 40
    double radius = 0.;
    // Code 50
    double startAngle = 0; // degrees
    // Code 51
    double endAngle = 0; // degrees
};

struct Dxf_CIRCLE : public Dxf_BaseGeom2dEntity {
    // Code 10, 20, 30
    DxfCoords centerPoint = {};
    // Code 40
    double radius = 0.;
};

struct Dxf_ELLIPSE : public Dxf_BaseGeom2dEntity {
    // Code 10, 20, 30
    DxfCoords centerPoint = {};
    // Code 11, 21, 31
    DxfCoords majorAxisEndPoint = {};
    // Code 40
    double ratioMinorMajorAxis = 0.;
    // Code 41
    double startParam = 0.;
    // Code 42
    double endParam = 2 * 3.14159265358979323846;
};
