// dxf.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
// modified 2018 wandererfan

// MAYO: file taken from FreeCad/src/Mod/Import/App/dxf.h -- commit #55292e9

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iosfwd>
#include <optional>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "freecad.h"

typedef int ColorIndex_t;  // DXF color index

typedef enum
{
    eUnspecified = 0,   // Unspecified (No units)
    eInches,
    eFeet,
    eMiles,
    eMillimeters,
    eCentimeters,
    eMeters,
    eKilometers,
    eMicroinches,
    eMils,
    eYards,
    eAngstroms,
    eNanometers,
    eMicrons,
    eDecimeters,
    eDekameters,
    eHectometers,
    eGigameters,
    eAstronomicalUnits,
    eLightYears,
    eParsecs
} eDxfUnits_t;

struct DxfCoords {
    double x;
    double y;
    double z;
};

struct DxfScale {
    double x;
    double y;
    double z;
};

struct Dxf_STYLE {
    // Code: 2
    std::string name;
    // Code: 40
    double fixedTextHeight = 0;
    // Code: 41
    double widthFactor = 1.;
    // Code: 50
    // AutoCad documentation doesn't specify units, but "Group Codes in Numerical Order" section
    // states that codes 50-58 are in degrees
    double obliqueAngle = 0.;
    // Code: 3
    std::string primaryFontFileName;
    // Code: 4
    std::string bigFontFileName;

    // TODO Code 70(standard flag values)
    // TODO Code 71(text generation flags)
    // TODO Code 42(last height used)
};

struct Dxf_TEXT {
    // Code: 39
    double thickness = 0.;
    // Code: 10, 20, 30
    DxfCoords firstAlignmentPoint = {};
    // Code: 40
    double height = 0.;
    // Code: 1
    std::string str;
    // Code: 50
    // AutoCad documentation doesn't specify units, but "Group Codes in Numerical Order" section
    // states that codes 50-58 are in degrees
    double rotationAngle = 0.;
    // Code: 41
    // "This value is also adjusted when fit-type text is used"
    double relativeXScaleFactorWidth = 1.;
    // Code: 51
    // AutoCad documentation doesn't specify units, but "Group Codes in Numerical Order" section
    // states that codes 50-58 are in degrees
    double obliqueAngle = 0.;
    // Code: 7
    std::string styleName;

    // TODO Code 71(text generation flags)

    enum class HorizontalJustification {
        Left = 0, Center = 1, Right = 2, Aligned = 3, Middle = 4, Fit = 5
    };
    // Code: 72
    HorizontalJustification horizontalJustification = HorizontalJustification::Left;
    // Code: 11, 21, 31
    DxfCoords secondAlignmentPoint = {};
    // Code: 210, 220, 230
    DxfCoords extrusionDirection = {0., 0., 1.};

    enum class VerticalJustification {
        Baseline = 0, Bottom = 1, Middle = 2, Top = 3
    };
    // Code: 73
    VerticalJustification verticalJustification = VerticalJustification::Baseline;
};

struct Dxf_MTEXT {
    enum class AttachmentPoint {
        TopLeft = 1, TopCenter,    TopRight,
        MiddleLeft,  MiddleCenter, MiddleRight,
        BottomLeft,  BottomCenter, BottomRight
    };

    // Code: 10, 20, 30
    DxfCoords insertionPoint = {};
    // Code: 40
    double height = 0.;
    // Code: 41
    double referenceRectangleWidth = 0;
    // Code 71
    AttachmentPoint attachmentPoint = AttachmentPoint::TopLeft;

    // TODO Code 72(drawing direction)

    // Code: 1, 3
    std::string str;

    // TODO Code 7(text sytle name)

    // Code: 210, 220, 230
    DxfCoords extrusionDirection = {0., 0., 1.};

    // Code: 11, 21, 31
    DxfCoords xAxisDirection = {1., 0., 0.}; // WCS

    // NOTE AutoCad documentation states that codes 42, 43 are "read-only, ignored if supplied"

    double rotationAngle = 0.; // radians(AutoCad documentation)

    // TODO Code 73(line spacing style)
    // TODO Code 44(line spacing factor)
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

struct Dxf_VERTEX {
    enum class Bulge { StraightSegment = 0, SemiCircle = 1 };
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

    // Code: 10, 20, 30
    DxfCoords point = {};
    // Code: 40
    double startingWidth = 0.;
    // Code: 41
    double endingWidth = 0.;
    // Code: 42
    Bulge bulge = Bulge::StraightSegment;
    // Code: 70
    Flags flags = Flag::None;
    // Code: 50
    double curveFitTangentDirection = 0.;
    // Code: 71
    int polyfaceMeshVertex1 = 0;
    // Code: 72
    int polyfaceMeshVertex2 = 0;
    // Code: 73
    int polyfaceMeshVertex3 = 0;
    // Code: 74
    int polyfaceMeshVertex4 = 0;
    // Code: 91
    // int identifier = 0;
};

struct Dxf_POLYLINE {
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

    enum Type {
        NoSmoothSurfaceFitted = 0,
        QuadraticBSplineSurface = 5,
        CubicBSplineSurface = 6,
        BezierSurface = 8
    };

    // Code: 39
    double thickness = 0.;
    // Code: 70
    Flags flags = Flag::None;
    // Code: 40
    double defaultStartWidth = 0.;
    // Code: 41
    double defaultEndWidth = 0.;
    // Code: 71(number of vertices in the mesh)
    int polygonMeshMVertexCount = 0;
    // Code: 72(number of faces in the mesh)
    int polygonMeshNVertexCount = 0;
    // Code: 73
    double smoothSurfaceMDensity = 0.;
    // Code: 74
    double smoothSurfaceNDensity = 0.;
    // Code: 75
    Type type = Type::NoSmoothSurfaceFitted;
    // Code: 210, 220, 230
    DxfCoords extrusionDirection = { 0., 0., 1. };

    std::vector<Dxf_VERTEX> vertices;
};

struct Dxf_INSERT {
    // Code: 2
    std::string blockName;
    // Code: 10, 20, 30
    DxfCoords insertPoint = {}; // OCS
    // Code: 41, 42, 43
    DxfScale scaleFactor = { 1., 1., 1. };
    // Code: 50
    double rotationAngle = 0.;
    // Code: 70
    int columnCount = 1;
    // Code: 71
    int rowCount = 1;
    // Code: 44
    double columnSpacing = 0.;
    // Code: 45
    double rowSpacing = 0.;
    // Code: 210, 220, 230
    DxfCoords extrusionDirection = { 0., 0., 1. };
};

struct Dxf_QuadBase {
    // Code: 10, 20, 30
    DxfCoords corner1;
    // Code: 11, 21, 31
    DxfCoords corner2;
    // Code: 12, 22, 32
    DxfCoords corner3;
    // Code: 13, 23, 33
    DxfCoords corner4;
    bool hasCorner4 = false;
};

struct Dxf_3DFACE : public Dxf_QuadBase {
    enum Flag {
        None = 0,
        InvisibleEdge1 = 1,
        InvisibleEdge2 = 2,
        InvisibleEdge3 = 4,
        InvisibleEdge4 = 8
    };
    using Flags = unsigned;

    // Code: 70
    Flags flags = Flag::None;
};

struct Dxf_SOLID : public Dxf_QuadBase {
    // Code: 39
    double thickness = 0.;
    // Code: 210, 220, 230
    DxfCoords extrusionDirection = { 0., 0., 1. };
};

struct Dxf_SPLINE {
    enum Flag {
        None = 0,
        Closed = 1,
        Periodic = 2,
        Rational = 4,
        Planar = 8,
        Linear = 16 // Planar bit is also set
    };
    using Flags = unsigned;

    // Code: 210, 220, 230
    DxfCoords normalVector = { 0., 0., 1. };
    // Code: 70
    Flags flags = Flag::None;
    // Code: 71
    int degree = 0;

    // Code: 42
    double knotTolerance = 0.0000001;
    // Code: 43
    double controlPointTolerance = 0.0000001;
    // Code: 44
    double fitTolerance = 0.0000000001;

    // Code: 12, 22, 32
    std::vector<DxfCoords> startTangents;
    // Code: 13, 23, 33
    std::vector<DxfCoords> endTangents;
    // Code: 40
    std::vector<double> knots;
    // Code: 41
    std::vector<double> weights;
    // Code: 10, 20, 30
    std::vector<DxfCoords> controlPoints;
    // Code: 11, 21, 31
    std::vector<DxfCoords> fitPoints;
};

typedef enum
{
    RUnknown,
    ROlder,
    R10,
    R11_12,
    R13,
    R14,
    R2000,
    R2004,
    R2007,
    R2010,
    R2013,
    R2018,
    RNewer,
} eDXFVersion_t;

#if 0
//***************************
//data structures for writing
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
struct point3D
{
    double x;
    double y;
    double z;
};

struct SplineDataOut
{
    point3D norm;
    int degree;
    int knots;
    int control_points;
    int fit_points;
    int flag;
    point3D starttan;
    point3D endtan;
    std::vector<double> knot;
    std::vector<double> weight;
    std::vector<point3D> control;
    std::vector<point3D> fit;
};

struct LWPolyDataOut
{
    double nVert;
    int    Flag;
    double Width;
    double Elev;
    double Thick;
    std::vector<point3D> Verts;
    std::vector<double> StartWidth;
    std::vector<double> EndWidth;
    std::vector<double> Bulge;
    point3D Extr;
};
//********************

class CDxfWrite
{
private:
    std::ofstream m_ofs;
    bool m_fail;
    std::ostringstream m_ssBlock;
    std::ostringstream m_ssBlkRecord;
    std::ostringstream m_ssEntity;
    std::ostringstream m_ssLayer;

protected:
    void putLine(const Base::Vector3d& s,
                 const Base::Vector3d& e,
                 std::ostringstream& outStream,
                 const std::string& handle,
                 const std::string& ownerHandle);
    void putText(const char* text,
                 const Base::Vector3d& location1,
                 const Base::Vector3d& location2,
                 const double height,
                 const int horizJust,
                 std::ostringstream& outStream,
                 const std::string& handle,
                 const std::string& ownerHandle);
    void putArrow(const Base::Vector3d& arrowPos,
                  const Base::Vector3d& barb1Pos,
                  const Base::Vector3d& barb2Pos,
                  std::ostringstream& outStream,
                  const std::string& handle,
                  const std::string& ownerHandle);

    //! copy boiler plate file
    std::string getPlateFile(const std::string& fileSpec);
    void setDataDir(const std::string& s)
    {
        m_dataDir = s;
    }
    std::string getHandle();
    std::string getEntityHandle();
    std::string getLayerHandle();
    std::string getBlockHandle();
    std::string getBlkRecordHandle();

    std::string m_optionSource;
    int m_version;
    int m_handle;
    int m_entityHandle;
    int m_layerHandle;
    int m_blockHandle;
    int m_blkRecordHandle;
    bool m_polyOverride;
    
    std::string m_saveModelSpaceHandle;
    std::string m_savePaperSpaceHandle;
    std::string m_saveBlockRecordTableHandle;
    std::string m_saveBlkRecordHandle;
    std::string m_currentBlock;
    std::string m_dataDir;
    std::string m_layerName;
    std::vector<std::string> m_layerList;
    std::vector<std::string> m_blockList;
    std::vector<std::string> m_blkRecordList;

public:
    CDxfWrite(const char* filepath);
    ~CDxfWrite();
    
    void init();
    void endRun();

    bool Failed()
    {
        return m_fail;
    }
//    void setOptions(void);
//    bool isVersionValid(int vers);
    std::string getLayerName()
    {
        return m_layerName;
    }
    void setLayerName(std::string s);
    void setVersion(int v)
    {
        m_version = v;
    }
    void setPolyOverride(bool b)
    {
        m_polyOverride = b;
    }
    void addBlockName(std::string s, std::string blkRecordHandle);

    void writeLine(const double* s, const double* e);
    void writePoint(const double*);
    void writeArc(const double* s, const double* e, const double* c, bool dir);
    void writeEllipse(const double* c,
                      double major_radius,
                      double minor_radius,
                      double rotation,
                      double start_angle,
                      double end_angle,
                      bool endIsCW);
    void writeCircle(const double* c, double radius );
    void writeSpline(const SplineDataOut& sd);
    void writeLWPolyLine(const LWPolyDataOut& pd);
    void writePolyline(const LWPolyDataOut& pd);
    void writeVertex(double x, double y, double z);
    void writeText(const char* text,
                   const double* location1,
                   const double* location2,
                   const double height,
                   const int horizJust);
    void writeLinearDim(const double* textMidPoint,
                        const double* lineDefPoint,
                        const double* extLine1,
                        const double* extLine2,
                        const char* dimText,
                        int type);
    void writeLinearDimBlock(const double* textMidPoint,
                             const double* lineDefPoint,
                             const double* extLine1,
                             const double* extLine2,
                             const char* dimText,
                             int type);
    void writeAngularDim(const double* textMidPoint,
                         const double* lineDefPoint,
                         const double* startExt1,
                         const double* endExt1,
                         const double* startExt2,
                         const double* endExt2,
                         const char* dimText);
    void writeAngularDimBlock(const double* textMidPoint,
                              const double* lineDefPoint,
                              const double* startExt1,
                              const double* endExt1,
                              const double* startExt2,
                              const double* endExt2,
                              const char* dimText);
   void writeRadialDim(const double* centerPoint,
                       const double* textMidPoint,
                       const double* arcPoint,
                       const char* dimText);
    void writeRadialDimBlock(const double* centerPoint,
                            const double* textMidPoint,
                            const double* arcPoint,
                            const char* dimText);
    void writeDiametricDim(const double* textMidPoint,
                           const double* arcPoint1,
                           const double* arcPoint2,
                           const char* dimText);
    void writeDiametricDimBlock(const double* textMidPoint,
                                const double* arcPoint1,
                                const double* arcPoint2,
                                const char* dimText);

    void writeDimBlockPreamble();
    void writeBlockTrailer();

    void writeHeaderSection();
    void writeTablesSection();
    void writeBlocksSection();
    void writeEntitiesSection();
    void writeObjectsSection();
    void writeClassesSection();

    void makeLayerTable();
    void makeBlockRecordTableHead();
    void makeBlockRecordTableBody();
    void makeBlockSectionHead();
};
#endif

namespace DxfPrivate {

enum class StringToErrorMode { Throw = 0x1, ReturnErrorValue = 0x2 };

double stringToDouble(
    const std::string& line,
    StringToErrorMode errorMode = StringToErrorMode::Throw
);

int stringToInt(
    const std::string& line,
    StringToErrorMode errorMode = StringToErrorMode::Throw
);

unsigned stringToUnsigned(
    const std::string& line,
    StringToErrorMode errorMode = StringToErrorMode::Throw
);

} // namespace DxfPrivate

// derive a class from this and implement it's virtual functions
class CDxfRead
{
private:
    std::ifstream m_ifs;

    bool m_fail = false;
    std::string m_str;
    std::string m_unused_line;
    eDxfUnits_t m_eUnits = eMillimeters;
    bool m_measurement_inch = false;
    std::string m_layer_name{"0"}; // Default layer name
    std::string m_section_name;
    std::string m_block_name;
    bool m_ignore_errors = true;

    std::streamsize m_gcount = 0;
    int m_line_nb = 0;

    // Mapping from layer name -> layer color index
    std::unordered_map<std::string, ColorIndex_t> m_layer_ColorIndex_map;
    const ColorIndex_t ColorBylayer = 256;

    // Map styleName to Style object
    std::unordered_map<std::string, Dxf_STYLE> m_mapStyle;

    bool ReadInsUnits();
    bool ReadMeasurement();
    bool ReadAcadVer();
    bool ReadDwgCodePage();

    bool ReadLayer();
    bool ReadStyle();
    bool ReadLine();
    bool ReadMText();
    bool ReadText();
    bool ReadArc();
    bool ReadCircle();
    bool ReadEllipse();
    bool ReadPoint();
    bool ReadSpline();
    bool ReadLwPolyLine();
    bool ReadPolyLine();
    bool ReadVertex(Dxf_VERTEX* vertex);
    bool Read3dFace();
    bool ReadSolid();
    bool ReadSection();
    bool ReadTable();
    bool ReadEndSec();

    void OnReadArc(
        double start_angle,
        double end_angle,
        double radius,
        const DxfCoords& c,
        double z_extrusion_dir,
        bool hidden
    );
    void OnReadCircle(const DxfCoords& c, double radius, bool hidden);
    void OnReadEllipse(
        const DxfCoords& c,
        const DxfCoords& m,
        double ratio,
        double start_angle,
        double end_angle
    );
    bool ReadInsert();
    bool ReadDimension();
    bool ReadBlockInfo();

    bool ResolveEncoding();

    template<unsigned XCode = 10, unsigned YCode = 20, unsigned ZCode = 30>
    void HandleCoordCode(int n, DxfCoords* coords)
    {
        switch (n) {
        case XCode:
            coords->x = mm(DxfPrivate::stringToDouble(m_str));
            break;
        case YCode:
            coords->y = mm(DxfPrivate::stringToDouble(m_str));
            break;
        case ZCode:
            coords->z = mm(DxfPrivate::stringToDouble(m_str));
            break;
        }
    }

    template<unsigned XCode, unsigned YCode, unsigned ZCode>
    void HandleVectorCoordCode(int n, std::vector<DxfCoords>* ptrVecCoords)
    {
        if (n == XCode || ptrVecCoords->empty())
            ptrVecCoords->push_back({});

        HandleCoordCode<XCode, YCode, ZCode>(n, &ptrVecCoords->back());
    }

    void HandleCommonGroupCode(int n);

    void put_line(const std::string& value);
    void ResolveColorIndex();

    void ReportError_readInteger(const char* context);

protected:
    ColorIndex_t m_ColorIndex = 0;
    eDXFVersion_t m_version = RUnknown;  // Version from $ACADVER variable in DXF

    std::streamsize gcount() const;
    virtual void get_line();
    virtual void ReportError(const std::string& /*msg*/) {}

    virtual bool setSourceEncoding(const std::string& /*codepage*/) { return true; }
    virtual std::string toUtf8(const std::string& strSource) { return strSource; }

private:
    std::string m_CodePage;  // Code Page name from $DWGCODEPAGE or null if none/not read yet

public:
    CDxfRead(const char* filepath); // this opens the file
    virtual ~CDxfRead(); // this closes the file

    bool IgnoreErrors() const { return m_ignore_errors; }
    bool Failed() const { return m_fail; }

    double mm(double value) const;

    const Dxf_STYLE* findStyle(const std::string& name) const;

    void DoRead(bool ignore_errors = false); // this reads the file and calls the following functions

    virtual void OnReadLine(const DxfCoords& s, const DxfCoords& e, bool hidden) = 0;

    virtual void OnReadPolyline(const Dxf_POLYLINE&) = 0;

    virtual void OnRead3dFace(const Dxf_3DFACE&) = 0;

    virtual void OnReadPoint(const DxfCoords& s) = 0;

    virtual void OnReadText(const Dxf_TEXT&) = 0;

    virtual void OnReadMText(const Dxf_MTEXT&) = 0;

    virtual void OnReadArc(
        const DxfCoords& s, const DxfCoords& e, const DxfCoords& c, bool dir, bool hidden
    ) = 0;

    virtual void OnReadCircle(const DxfCoords& s, const DxfCoords& c, bool dir, bool hidden) = 0;

    virtual void OnReadEllipse(
        const DxfCoords& c,
        double major_radius,
        double minor_radius,
        double rotation,
        double start_angle,
        double end_angle,
        bool dir
    ) = 0;

    virtual void OnReadSpline(const Dxf_SPLINE& spline) = 0;

    virtual void OnReadInsert(const Dxf_INSERT& ins) = 0;

    virtual void OnReadSolid(const Dxf_SOLID& solid) = 0;

    virtual void OnReadDimension(
        const DxfCoords& s,
        const DxfCoords& e,
        const DxfCoords& point,
        double rotation
    ) = 0;

    virtual void AddGraphics() const = 0;

    std::string LayerName() const;
};
