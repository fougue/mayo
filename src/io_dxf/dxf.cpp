/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
//required by windows for M_PI definition
#  define _USE_MATH_DEFINES
#endif

#include "../base/libfromchars.h"
#include "dxf.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <clocale>
#include <cmath>
#include <locale>
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <gsl/util>

namespace {

class ScopedCLocale {
public:
    ScopedCLocale(int category)
        : m_category(category),
          m_savedLocale(std::setlocale(category, nullptr))
    {
        std::setlocale(category, "C");
    }

    ~ScopedCLocale()
    {
        std::setlocale(m_category, m_savedLocale);
    }

private:
    int m_category = 0;
    const char* m_savedLocale = nullptr;
};

} // namespace

namespace DxfPrivate {

template<typename T> bool isStringToErrorValue(T value)
{
    if constexpr(std::is_same_v<T, int>) {
        return value == std::numeric_limits<int>::max();
    }
    else if constexpr(std::is_same_v<T, unsigned>) {
        return value == std::numeric_limits<unsigned>::max();
    }
    else if constexpr(std::is_same_v<T, double>) {
        constexpr double dMax = std::numeric_limits<double>::max();
        return std::abs(value - dMax) * 1000000000000. <= std::min(std::abs(value), std::abs(dMax));
    }
    else {
        return false;
    }
}

template<typename T>
T stringToNumeric(const std::string& line, StringToErrorMode errorMode)
{
    T value;
    auto [ptr, err] = Mayo::fromChars(line, value);
    if (err == std::errc())
        return value;

    if (errorMode == StringToErrorMode::ReturnErrorValue) {
        return std::numeric_limits<T>::max();
    }
    else {
        std::string strTypeName = "?";
        if constexpr(std::is_same_v<T, int>)
            strTypeName = "int";
        else if constexpr(std::is_same_v<T, unsigned>)
            strTypeName = "unsigned";
        else if constexpr(std::is_same_v<T, double>)
            strTypeName = "double";

        throw std::runtime_error("Failed to fetch " + strTypeName + " value from line:\n" + line);
    }
}

int stringToInt(const std::string& line, StringToErrorMode errorMode)
{
    return stringToNumeric<int>(line, errorMode);
}

unsigned stringToUnsigned(const std::string& line, StringToErrorMode errorMode)
{
    return stringToNumeric<unsigned>(line, errorMode);
}

double stringToDouble(const std::string& line, StringToErrorMode errorMode)
{
    return stringToNumeric<double>(line, errorMode);
}

} // namespace DxfPrivate

using namespace DxfPrivate;

CDxfRead::CDxfRead()
{
    m_mapEntityHandler.insert({ "ARC", [=]{ return ReadArc(); } });
    m_mapEntityHandler.insert({ "BLOCK", [=]{ return ReadBlock(); } });
    m_mapEntityHandler.insert({ "CIRCLE", [=]{ return ReadCircle(); } });
    m_mapEntityHandler.insert({ "DIMENSION", [=]{ return ReadDimension(); } });
    m_mapEntityHandler.insert({ "ELLIPSE", [=]{ return ReadEllipse(); } });
    m_mapEntityHandler.insert({ "INSERT", [=]{ return ReadInsert(); } });
    m_mapEntityHandler.insert({ "LAYER", [=]{ return ReadLayer(); } });
    m_mapEntityHandler.insert({ "LINE", [=]{ return ReadLine(); } });
    m_mapEntityHandler.insert({ "LWPOLYLINE", [=]{ return ReadLwPolyLine(); } });
    m_mapEntityHandler.insert({ "MTEXT", [=]{ return ReadMText(); } });
    m_mapEntityHandler.insert({ "POINT", [=]{ return ReadPoint(); } });
    m_mapEntityHandler.insert({ "POLYLINE", [=]{ return ReadPolyLine(); } });
    m_mapEntityHandler.insert({ "SECTION", [=]{ return ReadSection(); } });
    m_mapEntityHandler.insert({ "SOLID", [=]{ return ReadSolid(); } });
    m_mapEntityHandler.insert({ "3DFACE", [=]{ return Read3dFace(); } });
    m_mapEntityHandler.insert({ "SPLINE", [=]{ return ReadSpline(); } });
    m_mapEntityHandler.insert({ "STYLE", [=]{ return ReadStyle(); } });
    m_mapEntityHandler.insert({ "TEXT", [=]{ return ReadText(); } });
    m_mapEntityHandler.insert({ "TABLE", [=]{ return ReadTable(); } });
    m_mapEntityHandler.insert({ "ENDSEC", [=]{ return ReadEndSec(); } });
}

CDxfRead::~CDxfRead()
{
}

double CDxfRead::mm(double value) const
{
    // re #6461
    // this if handles situation of malformed DXF file where
    // MEASUREMENT specifies English units, but
    // INSUNITS specifies millimeters or is not specified
    //(millimeters is our default)
    if (m_measurement_inch && (m_unit == DxfUnit::Millimeters)) {
        value *= 25.4;
    }

    switch (m_unit) {
    case DxfUnit::Unspecified:
        return (value * 1.0);  // We don't know any better
    case DxfUnit::Inches:
        return (value * 25.4);
    case DxfUnit::Feet:
        return (value * 25.4 * 12);
    case DxfUnit::Miles:
        return (value * 1609344.0);
    case DxfUnit::Millimeters:
        return (value * 1.0);
    case DxfUnit::Centimeters:
        return (value * 10.0);
    case DxfUnit::Meters:
        return (value * 1000.0);
    case DxfUnit::Kilometers:
        return (value * 1000000.0);
    case DxfUnit::Microinches:
        return (value * 25.4 / 1000.0);
    case DxfUnit::Mils:
        return (value * 25.4 / 1000.0);
    case DxfUnit::Yards:
        return (value * 3 * 12 * 25.4);
    case DxfUnit::Angstroms:
        return (value * 0.0000001);
    case DxfUnit::Nanometers:
        return (value * 0.000001);
    case DxfUnit::Microns:
        return (value * 0.001);
    case DxfUnit::Decimeters:
        return (value * 100.0);
    case DxfUnit::Dekameters:
        return (value * 10000.0);
    case DxfUnit::Hectometers:
        return (value * 100000.0);
    case DxfUnit::Gigameters:
        return (value * 1000000000000.0);
    case DxfUnit::AstronomicalUnits:
        return (value * 149597870690000.0);
    case DxfUnit::LightYears:
        return (value * 9454254955500000000.0);
    case DxfUnit::Parsecs:
        return (value * 30856774879000000000.0);
    default:
        return (value * 1.0);  // We don't know any better
    }
}

bool CDxfRead::hasHeaderVariable(std::string_view name) const
{
    return m_mapHeaderVarValue.find(name) != m_mapHeaderVarValue.cend();
}

Dxf_HeaderVariableValue CDxfRead::headerVariableValue(std::string_view name) const
{
    auto it = m_mapHeaderVarValue.find(name);
    return it != m_mapHeaderVarValue.cend() ? it->second : Dxf_HeaderVariableValue{};
}

const Dxf_BLOCK* CDxfRead::findBlock(DxfStringRef name) const
{
    auto it = m_mapBlock.find(name);
    return it != m_mapBlock.cend() ? it->second : nullptr;
}

const Dxf_LAYER* CDxfRead::findLayer(DxfStringRef name) const
{
    auto it = m_mapLayer.find(name);
    return it != m_mapLayer.cend() ? it->second : nullptr;
}

const Dxf_STYLE* CDxfRead::findStyle(DxfStringRef name) const
{
    auto it = m_mapStyle.find(name);
    return it != m_mapStyle.cend() ? it->second : nullptr;
}

bool CDxfRead::readEntity(
        const std::function<void()>& fnEntityHandler,
        const std::function<void(int)>& fnCodeHandler,
        std::string_view entityTypeName
    )
{
    while (!inputStream().eof()) {
        getLine();
        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (n == 0) {
            // Next item found, so finish with entity
            fnEntityHandler();
            return true;
        }
        else if (isStringToErrorValue(n)) {
            std::string context;
            context += "DXF::Read";
            context += entityTypeName;
            context += "()";
            this->ReportError_readInteger(context.c_str());
            return false;
        }

        getLine();
        fnCodeHandler(n);
    }

    fnEntityHandler();
    return false;
}

bool CDxfRead::ReadLine()
{
    Dxf_LINE line;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(line), m_lines);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            handleCoordCode(n, &line.startPoint);
            break;
        case 11: case 21: case 31:
            handleCoordCode<11, 21, 31>(n, &line.endPoint);
            break;
        default:
            this->handleCommonGroupCode(&line, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Line");
}

bool CDxfRead::ReadPoint()
{
    Dxf_POINT point;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(point), m_points);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            handleCoordCode(n, &point.location);
            break;
        case 50:
            point.angleXAxis = stringToDouble(m_str);
            break;
        default:
            this->handleCommonGroupCode(&point, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Point");
}

bool CDxfRead::ReadArc()
{
    Dxf_ARC arc;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(arc), m_arcs);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            handleCoordCode(n, &arc.centerPoint);
            break;
        case 40:
            arc.radius = mm(stringToDouble(m_str));
            break;
        case 50:
            arc.startAngle = stringToDouble(m_str);
            break;
        case 51:
            arc.endAngle = stringToDouble(m_str);
            break;
        default:
            this->handleCommonGroupCode(&arc, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Arc");
}

bool CDxfRead::ReadSpline()
{
    int knotCount = 0;
    int controlPointCount = 0;
    int fitPointCount = 0;
    Dxf_SPLINE spline;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(spline), m_splines);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 210: case 220: case 230:
            this->handleCoordCode<210, 220, 230>(n, &spline.normalVector);
            break;
        case 70:
            spline.flags = stringToUnsigned(m_str);
            break;
        case 71:
            spline.degree = stringToInt(m_str);
            break;
        case 72:
            knotCount = stringToInt(m_str);
            spline.knots.reserve(knotCount);
            break;
        case 73:
            controlPointCount = stringToInt(m_str);
            spline.controlPoints.reserve(controlPointCount);
            break;
        case 74:
            fitPointCount = stringToInt(m_str);
            spline.fitPoints.reserve(fitPointCount);
            break;
        case 12: case 22: case 32:
            this->handleCoordCode<12, 22, 32>(n, &spline.startTangent);
            break;
        case 13: case 23: case 33:
            this->handleCoordCode<13, 23, 33>(n, &spline.endTangent);
            break;
        case 40:
            spline.knots.push_back(mm(stringToDouble(m_str)));
            break;
        case 41:
            spline.weights.push_back(mm(stringToDouble(m_str)));
            break;
        case 10: case 20: case 30:
            this->handleVectorCoordCode<10, 20, 30>(n, &spline.controlPoints);
            break;
        case 11: case 21: case 31:
            this->handleVectorCoordCode<11, 21, 31>(n, &spline.fitPoints);
            break;
        case 42:
            spline.knotTolerance = stringToDouble(m_str);
            break;
        case 43:
            spline.controlPointTolerance = stringToDouble(m_str);
            break;
        case 44:
            spline.fitTolerance = stringToDouble(m_str);
            break;
        default:
            this->handleCommonGroupCode(&spline, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Spline");
}

bool CDxfRead::ReadCircle()
{
    Dxf_CIRCLE circle;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(circle), m_circles);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            this->handleCoordCode(n, &circle.centerPoint);
            break;
        case 40:
            circle.radius = mm(stringToDouble(m_str));
            break;
        default:
            this->handleCommonGroupCode(&circle, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Circle");
}

bool CDxfRead::ReadMText()
{
    Dxf_MTEXT text;
    bool withinAcadColumnInfo = false;
    bool withinAcadColumns = false;
    bool withinAcadDefinedHeight = false;
    std::string strText;

    auto fnMatchExtensionBegin = [=](std::string_view extName, bool& tag) {
        if (!tag && m_str == extName) {
            tag = true;
            return true;
        }
        return false;
    };
    auto fnMatchExtensionEnd = [=](std::string_view extName, bool& tag) {
        if (tag && m_str == extName) {
            tag = false;
            return true;
        }
        return false;
    };

    auto fnEntityHandler = [&]{
        // Replace \P by \n
        size_t pos = strText.find("\\P", 0);
        while (pos != std::string::npos) {
            strText.replace(pos, 2, "\n");
            pos = strText.find("\\P", pos + 1);
        }

        text.str = m_strCache.add(this->toUtf8(strText));
        this->addEntity(std::move(text), m_mtexts);
    };
    auto fnCodeHandler = [&](int n) {
        if (fnMatchExtensionBegin("ACAD_MTEXT_COLUMN_INFO_BEGIN", withinAcadColumnInfo)) {
            text.acadHasColumnInfo = true;
            return; // Skip
        }

        if (fnMatchExtensionEnd("ACAD_MTEXT_COLUMN_INFO_END", withinAcadColumnInfo))
            return; // Skip

        if (fnMatchExtensionBegin("ACAD_MTEXT_COLUMNS_BEGIN", withinAcadColumns))
            return; // Skip

        if (fnMatchExtensionEnd("ACAD_MTEXT_COLUMNS_END", withinAcadColumns))
            return; // Skip

        if (fnMatchExtensionBegin("ACAD_MTEXT_DEFINED_HEIGHT_BEGIN", withinAcadDefinedHeight)) {
            text.acadHasDefinedHeight = true;
            return; // Skip
        }

        if (fnMatchExtensionEnd("ACAD_MTEXT_DEFINED_HEIGHT_END", withinAcadDefinedHeight))
            return; // Skip

        if (withinAcadColumnInfo) {
            // 1040/1070 extended data code was found at beginning of current iteration
            const int xn = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
            getLine(); // Skip 1040/1070 line
            getLine(); // Get value line of extended data code
            switch (xn) {
            case 75: { // 1070
                const int t = stringToInt(m_str);
                if (0 <= t && t <= 2)
                    text.acadColumnInfo_Type = static_cast<Dxf_MTEXT::ColumnType>(t);
            }
                break;
            case 76: // 1070
                text.acadColumnInfo_Count = stringToInt(m_str);
                break;
            case 78: // 1070
                text.acadColumnInfo_FlowReversed = stringToInt(m_str) != 0;
                break;
            case 79: // 1070
                text.acadColumnInfo_AutoHeight = stringToInt(m_str) != 0;
                break;
            case 48: // 1040
                text.acadColumnInfo_Width = mm(stringToDouble(m_str));
                break;
            case 49: // 1040
                text.acadColumnInfo_GutterWidth = mm(stringToDouble(m_str));
                break;
            } // endswitch

            return; // Skip
        }

        if (withinAcadDefinedHeight) {
            // 1040/1070 extended data code was found at beginning of current iteration
            const int xn = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
            getLine(); // Skip 1040/1070 line
            getLine(); // Get value line of extended data code
            if (xn == 46)
                text.acadDefinedHeight = mm(stringToDouble(m_str));

            return; // Skip
        }

        switch (n) {
        case 1:
            // Final text
            strText.append(m_str);
            break;
        case 3:
            // Additional text that goes before the type 1 text
            // Note that if breaking the text into type-3 records splits a UFT-8 encoding we do
            // the decoding after splicing the lines together. I'm not sure if this actually
            // occurs, but handling the text this way will treat this condition properly.
            strText.append(m_str);
            break;
        case 7:
            text.styleName = m_strCache.add(m_str);
            break;
        case 10: case 20: case 30:
            this->handleCoordCode(n, &text.insertionPoint);
            break;
        case 11: case 21: case 31:
            this->handleCoordCode<11, 21, 31>(n, &text.xAxisDirection);
            break;
        case 40:
            text.height = mm(stringToDouble(m_str));
            break;
        case 41:
            text.referenceRectangleWidth = stringToDouble(m_str);
            break;
        case 44:
            text.lineSpacingFactor = stringToDouble(m_str);
            break;
        case 50:
            text.rotationAngle = stringToDouble(m_str);
            break;
        case 71: {
            const int ap = stringToInt(m_str);
            if (ap >= 1 && ap <= 9)
                text.attachmentPoint = static_cast<Dxf_MTEXT::AttachmentPoint>(ap);
        }
            break;
        case 72:
            text.drawingDirection = stringToUnsigned(m_str);
            break;
        case 73:
            text.lineSpacingStyle = stringToUnsigned(m_str);
            break;
        default:
            this->handleCommonGroupCode(&text, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "MText");
}

bool CDxfRead::ReadText()
{
    Dxf_TEXT text;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(text), m_texts);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            handleCoordCode(n, &text.firstAlignmentPoint);
            break;
        case 40:
            text.height = mm(stringToDouble(m_str));
            break;
        case 1:
            text.str = m_strCache.add(this->toUtf8(m_str));
            break;
        case 50:
            text.rotationAngle = stringToDouble(m_str);
            break;
        case 41:
            text.relativeXScaleFactorWidth = stringToDouble(m_str);
            break;
        case 51:
            text.obliqueAngle = stringToDouble(m_str);
            break;
        case 7:
            text.styleName = m_strCache.add(m_str);
            break;
        case 71:
            text.generationFlags = stringToUnsigned(m_str);
            break;
        case 72: {
            const int hjust = stringToInt(m_str);
            if (hjust >= 0 && hjust <= 5)
                text.horizontalJustification = static_cast<Dxf_TEXT::HorizontalJustification>(hjust);
        }
            break;
        case 11: case 21: case 31:
            this->handleCoordCode<11, 21, 31>(n, &text.secondAlignmentPoint);
            break;
        case 210: case 220: case 230:
            this->handleCoordCode<210, 220, 230>(n, &text.extrusionDirection);
            break;
        case 73: {
            const int vjust = stringToInt(m_str);
            if (vjust >= 0 && vjust <= 3)
                text.verticalJustification = static_cast<Dxf_TEXT::VerticalJustification>(vjust);
        }
            break;
        default:
            this->handleCommonGroupCode(&text, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Text");
}

bool CDxfRead::ReadEllipse()
{
    Dxf_ELLIPSE ellipse;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(ellipse), m_ellipses);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            this->handleCoordCode(n, &ellipse.centerPoint);
            break;
        case 11: case 21: case 31:
            this->handleCoordCode<11, 21, 31>(n, &ellipse.majorAxisEndPoint);
            break;
        case 40:
            ellipse.ratioMinorMajorAxis = stringToDouble(m_str);
            break;
        case 41:
            ellipse.startParam = stringToDouble(m_str);
            break;
        case 42:
            ellipse.endParam = stringToDouble(m_str);
            break;
        default:
            this->handleCommonGroupCode(&ellipse, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Ellipse");
}

bool CDxfRead::ReadLwPolyLine()
{
    Dxf_LWPOLYLINE polyline;
    unsigned declaredSize = 0;
    Dxf_LWPOLYLINE::Vertex vertex;
    bool firstXCoordFound = false;
    auto fnEntityHandler = [&]{
        if (declaredSize > 0)
            polyline.vertices.push_back(vertex);
        this->addEntity(std::move(polyline), m_lwpolylines);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 38:
            polyline.elevation = mm(stringToDouble(m_str));
            break;
        case 43:
            polyline.constantWidth = stringToDouble(m_str);
            break;
        case 70:
            polyline.flag = stringToUnsigned(m_str);
            break;
        case 90:
            declaredSize = stringToUnsigned(m_str);
            polyline.vertices.reserve(declaredSize);
            break;
        case 10: { // Vertex
            if (firstXCoordFound) {
                polyline.vertices.push_back(vertex);
                vertex = {};
            }

            vertex.x = mm(stringToDouble(m_str));
            firstXCoordFound = true;
        }
            break;
        case 20: // Vertex
            vertex.y = mm(stringToDouble(m_str));
            break;
        case 40: // Vertex
            vertex.startWidth = stringToDouble(m_str);
            break;
        case 41: // Vertex
            vertex.endWidth = stringToDouble(m_str);
            break;
        case 42: // Vertex
            vertex.bulge = stringToDouble(m_str);
            break;
        default:
            this->handleCommonGroupCode(&polyline, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "LwPolyline");
}

bool CDxfRead::ReadVertex(Dxf_POLYLINE::Vertex* vertex)
{
    bool x_found = false;
    bool y_found = false;
    while (!inputStream().eof()) {
        getLine();
        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (n == 0) {
            // Read one line too many.  put it back.
            putLine(m_str);
            return x_found && y_found;
        }
        else if (isStringToErrorValue(n)) {
            this->ReportError_readInteger("DXF::ReadVertex()");
            return false;
        }

        getLine();
        switch (n){
        case 10: case 20: case 30:
            x_found = x_found || n == 10;
            y_found = y_found || n == 20;
            this->handleCoordCode(n, &vertex->point);
            break;
        case 40:
            vertex->startingWidth = stringToDouble(m_str);
            break;
        case 41:
            vertex->endingWidth = stringToDouble(m_str);
            break;
        case 42:
            vertex->bulge = stringToDouble(m_str);
            break;
        case 70:
            vertex->flags = stringToUnsigned(m_str);
            break;
        case 71:
            vertex->polyfaceMeshVertex1 = stringToInt(m_str);
            break;
        case 72:
            vertex->polyfaceMeshVertex2 = stringToInt(m_str);
            break;
        case 73:
            vertex->polyfaceMeshVertex3 = stringToInt(m_str);
            break;
        case 74:
            vertex->polyfaceMeshVertex4 = stringToInt(m_str);
            break;
        }
    }

    return false;
}

bool CDxfRead::Read3dFace()
{
    Dxf_3DFACE face;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(face), m_3dfaces);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            this->handleCoordCode<10, 20, 30>(n, &face.corner1);
            break;
        case 11: case 21: case 31:
            this->handleCoordCode<11, 21, 31>(n, &face.corner2);
            break;
        case 12: case 22: case 32:
            this->handleCoordCode<12, 22, 32>(n, &face.corner3);
            break;
        case 13: case 23: case 33:
            this->handleCoordCode<13, 23, 33>(n, &face.corner4);
            face.hasCorner4 = true;
            break;
        case 70:
            face.flags = stringToUnsigned(m_str);
            break;
        default:
            this->handleCommonGroupCode(&face, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "3DFace");
}

bool CDxfRead::ReadSolid()
{
    Dxf_SOLID solid;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(solid), m_solids);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 10: case 20: case 30:
            this->handleCoordCode<10, 20, 30>(n, &solid.corner1);
            break;
        case 11: case 21: case 31:
            this->handleCoordCode<11, 21, 31>(n, &solid.corner2);
            break;
        case 12: case 22: case 32:
            this->handleCoordCode<12, 22, 32>(n, &solid.corner3);
            break;
        case 13: case 23: case 33:
            this->handleCoordCode<13, 23, 33>(n, &solid.corner4);
            solid.hasCorner4 = true;
            break;
        case 39:
            solid.thickness = stringToDouble(m_str);
            break;
        case 210: case 220: case 230:
            this->handleCoordCode<210, 220, 230>(n, &solid.extrusionDirection);
            break;
        default:
            this->handleCommonGroupCode(&solid, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Solid");
}

bool CDxfRead::ReadPolyLine()
{
    Dxf_POLYLINE polyline;
    while (!inputStream().eof()) {
        getLine();
        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (isStringToErrorValue(n)) {
            this->ReportError_readInteger("DXF::ReadPolyLine()");
            return false;
        }

        getLine();
        switch (n) {
        case 0:
            if (m_str == "VERTEX") {
                Dxf_POLYLINE::Vertex vertex;
                if (ReadVertex(&vertex))
                    polyline.vertices.push_back(std::move(vertex));
            }
            else if (m_str == "SEQEND") {
                this->addEntity(std::move(polyline), m_polylines);
                return true;
            }

            break;
        case 70:
            polyline.flags = stringToUnsigned(m_str);
            break;
        case 40:
            polyline.defaultStartWidth = stringToDouble(m_str);
            break;
        case 41:
            polyline.defaultEndWidth = stringToDouble(m_str);
            break;
        case 71:
            polyline.polygonMeshMVertexCount = stringToInt(m_str);
            break;
        case 72:
            polyline.polygonMeshNVertexCount = stringToInt(m_str);
            break;
        case 73:
            polyline.smoothSurfaceMDensity = stringToDouble(m_str);
            break;
        case 74:
            polyline.smoothSurfaceNDensity = stringToDouble(m_str);
            break;
        case 75:
            polyline.polylineType = static_cast<Dxf_POLYLINE::PolylineType>(stringToUnsigned(m_str));
            break;
        default:
            this->handleCommonGroupCode(&polyline, n);
            break;
        }
    }

    return false;
}

bool CDxfRead::ReadInsert()
{
    Dxf_INSERT insert;
    auto fnEntityHandler = [&]{
        this->addEntity(std::move(insert), m_inserts);
    };
    auto fnCodeHandler = [&](int n) {
        switch (n) {
        case 2:
            insert.blockName = m_strCache.add(m_str);
            break;
        case 10: case 20: case 30:
            this->handleCoordCode(n, &insert.insertPoint);
            break;
        case 41:
            insert.scaleFactor.x = stringToDouble(m_str);
            break;
        case 42:
            insert.scaleFactor.y = stringToDouble(m_str);
            break;
        case 43:
            insert.scaleFactor.z = stringToDouble(m_str);
            break;
        case 50:
            insert.rotationAngle = stringToDouble(m_str);
            break;
        case 70:
            insert.columnCount = stringToInt(m_str);
            break;
        case 71:
            insert.rowCount = stringToInt(m_str);
            break;
        case 44:
            insert.columnSpacing = mm(stringToDouble(m_str));
            break;
        case 45:
            insert.rowSpacing = mm(stringToDouble(m_str));
            break;
        case 210: case 220: case 230:
            handleCoordCode<210, 220, 230>(n, &insert.extrusionDirection);
            break;
        default:
            this->handleCommonGroupCode(&insert, n);
            break;
        }
    };
    return this->readEntity(fnEntityHandler, fnCodeHandler, "Insert");
}

bool CDxfRead::ReadDimension()
{
    DxfCoords s = {}; // startpoint
    DxfCoords e = {}; // endpoint
    DxfCoords p = {}; // dimpoint
    double rot = -1.0; // rotation

    while (!inputStream().eof()) {
        getLine();
        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (n == 0) {
            // next item found
            //OnReadDimension(s, e, p, rot * M_PI/180);
            return true;
        }
        else if (isStringToErrorValue(n)) {
            this->ReportError_readInteger("DXF::ReadDimension()");
            return false;
        }

        getLine();
        switch (n){
        case 13: case 23: case 33:
            // start coords
            handleCoordCode<13, 23, 33>(n, &s);
            break;
        case 14: case 24: case 34:
            // end coords
            handleCoordCode<14, 24, 34>(n, &e);
            break;
        case 10: case 20: case 30:
            // dimline coords
            handleCoordCode<10, 20, 30>(n, &p);
            break;
        case 50:
            // rotation
            rot = stringToDouble(m_str);
            break;
        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
            // skip the next line
            break;
        default:
            //handleCommonGroupCode(n);
            break;
        }
    }

    return false;
}

bool CDxfRead::ReadBlock()
{
    Dxf_BLOCK block;
    m_currentBlock = &block;
    auto _ = gsl::finally([=]{ m_currentBlock = nullptr; });
    auto endBlockHandled = [&]{
        if (m_str == "ENDBLK") {
            m_blocks.add(std::move(block));
            m_mapBlock.insert({m_blocks.back().name, &m_blocks.back()});
            return true;
        }
        else {
            return false;
        }
    };
    while (!inputStream().eof()) {
        getLine();
        if (endBlockHandled())
            return true;

        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (isStringToErrorValue(n)) {
            this->ReportError_readInteger("DXF::ReadBlock()");
            return false;
        }

        getLine();
        switch (n) {
        case 0: {
            while (!endBlockHandled()) {
                auto itHandler = m_mapEntityHandler.find(m_str);
                if (itHandler != m_mapEntityHandler.cend()) {
                    const auto& fnEntityHandler = itHandler->second;
                    bool okRead = false;
                    try {
                        okRead = fnEntityHandler();
                    } catch (const std::runtime_error& err) {
                        this->ReportError(err.what());
                    }
                }

                getLine();
            }

            return true;
        }
            break;
        case 2: case 3:
            block.name = m_strCache.add(m_str);
            break;
        case 8:
            block.layerName = m_strCache.add(m_str);
            break;
        case 10: case 20: case 30:
            this->handleCoordCode(n, &block.basePoint);
            break;
        case 70:
            block.flags = stringToUnsigned(m_str);
            break;
        case 1:
            block.xrefPathName = m_strCache.add(m_str);
            break;
        case 4:
            block.description = m_strCache.add(m_str);
            break;
        default:
            break;
        }
    }

    return false;
}

bool CDxfRead::ReadSection()
{
    getLine();
    getLine();
    return true;
}

bool CDxfRead::ReadTable()
{
    getLine();
    getLine();
    return true;
}

bool CDxfRead::ReadEndSec()
{
    return true;
}

void CDxfRead::getLine()
{
    if (!m_unusedLine.empty()) {
        m_str = m_unusedLine;
        m_unusedLine.clear();
        return;
    }

    std::getline(inputStream(), m_str);
    m_gcount = m_str.size();

    // Erase leading whitespace characters
    auto itNonSpace = m_str.begin();
    while (itNonSpace != m_str.end()) {
        if (!std::isspace(*itNonSpace))
            break;

        ++itNonSpace;
    }

    m_str.erase(m_str.begin(), itNonSpace);
}

void CDxfRead::putLine(const std::string& value)
{
    m_unusedLine = value;
}

bool CDxfRead::ReadLayer()
{
    Dxf_LAYER layer;
    while (!inputStream().eof()) {
        getLine();
        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (n == 0) {
            if (layer.name.empty()) {
                this->ReportError_readInteger("DXF::ReadLayer() - no layer name");
                return false;
            }

            m_layers.add(std::move(layer));
            m_mapLayer.insert({m_layers.back().name, &m_layers.back()});
            return true;
        }
        else if (isStringToErrorValue(n)) {
            this->ReportError_readInteger("DXF::ReadLayer()");
            return false;
        }

        getLine();
        switch (n) {
        case 2:
            layer.name = m_strCache.add(m_str);
            break;
        case 6:
            layer.lineTypeName = m_strCache.add(m_str);
            break;
        case 62:
            layer.colorId = stringToInt(m_str);
            break;
        case 70:
            layer.flags = stringToUnsigned(m_str);
            break;
        default:
            break;
        }
    }

    return false;
}

bool CDxfRead::ReadStyle()
{
    Dxf_STYLE style;
    while (!inputStream().eof()) {
        getLine();
        const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
        if (n == 0) {
            if (style.name.empty()) {
                this->ReportError_readInteger("DXF::ReadStyle() - no style name");
                return false;
            }

            m_styles.add(std::move(style));
            m_mapStyle.insert({m_styles.back().name, &m_styles.back()});
            return true;
        }
        else if (isStringToErrorValue(n)) {
            this->ReportError_readInteger("DXF::ReadStyle()");
            return false;
        }

        getLine();
        switch (n) {
        case 2:
            style.name = m_strCache.add(m_str);
            break;
        case 40:
            style.fixedTextHeight = mm(stringToDouble(m_str));
            break;
        case 41:
            style.widthFactor = stringToDouble(m_str);
            break;
        case 50:
            style.obliqueAngle = stringToDouble(m_str);
            break;
        case 3:
            style.primaryFontFileName = m_strCache.add(m_str);
            break;
        case 4:
            style.bigFontFileName = m_strCache.add(m_str);
            break;
        default:
            break; // skip the next line
        }
    }

    return false;
}

void CDxfRead::resolveAcadVer(DxfStringRef strVersion)
{
    static const std::string_view versionNames[] = {
        // This table is indexed by eDXFVersion_t - (ROlder+1)
        "AC1006",
        "AC1009",
        "AC1012",
        "AC1014",
        "AC1015",
        "AC1018",
        "AC1021",
        "AC1024",
        "AC1027",
        "AC1032"
    };

    assert(
        std::size(versionNames)
        == (static_cast<int>(DxfVersion::RNewer) - static_cast<int>(DxfVersion::ROlder) - 1)
    );
    auto first = std::cbegin(versionNames);
    auto last = std::cend(versionNames);
    auto found = std::lower_bound(first, last, strVersion);
    if (found == last) {
        m_version = DxfVersion::RNewer;
    }
    else if (*found == strVersion) {
        m_version = static_cast<DxfVersion>(std::distance(first, found) + (static_cast<int>(DxfVersion::ROlder) + 1));
    }
    else if (found == first) {
        m_version = DxfVersion::ROlder;
    }
    else {
        m_version = DxfVersion::RUnknown;
    }

    this->resolveEncoding(m_version);
}

void CDxfRead::resolveEncoding(DxfVersion version)
{
    //
    // See https://ezdxf.readthedocs.io/en/stable/dxfinternals/fileencoding.html#
    //

    if (version >= DxfVersion::R2007) { // Note this does not include RUnknown, but does include RLater
        this->setSourceEncoding("UTF8");
    }
    else {
        std::transform(m_codePage.cbegin(), m_codePage.cend(), m_codePage.begin(), [](char c) {
            return std::toupper(c, std::locale::classic());
        });
        // ANSI_1252 by default if $DWGCODEPAGE is not set
        if (m_codePage.empty())
            m_codePage = "ANSI_1252";

        this->setSourceEncoding(m_codePage);
    }
}

void CDxfRead::handleCommonGroupCode(Dxf_BaseEntity* entity, int n)
{
    switch (n) {
    case 5:
        entity->handle = m_strCache.add(m_str);
        break;
    case 6:
        entity->lineTypeName = m_strCache.add(m_str);
        break;
    case 8:
        entity->layerName = m_strCache.add(m_str);
        break;
    case 60:
        entity->isVisible = (stringToInt(m_str) == 0);
        break;
    case 62:
        entity->colorId = stringToInt(m_str);
        break;
    case 67:
        entity->space = stringToInt(m_str) == 0 ? Dxf_BaseEntity::Space::Model : Dxf_BaseEntity::Space::Paper;
        break;
    }
}

void CDxfRead::handleCommonGroupCode(Dxf_BaseGeom2dEntity* entity, int n)
{
    this->handleCommonGroupCode(static_cast<Dxf_BaseEntity*>(entity), n);
    switch (n) {
    case 39:
        entity->thickness = stringToDouble(m_str);
        break;
    case 210: case 220: case 230:
        this->handleCoordCode<210, 220, 230>(n, &entity->extrusionDirection);
        break;
    }
}

void CDxfRead::readHeaderVariable()
{
    assert(!m_str.empty() && m_str.at(0) == '$');

    auto isXyzCoordGroupCode = [](int n) {
        return n == 10 || n == 20 || n == 30;
    };

    auto varName = m_strCache.add({&m_str.at(1), m_str.size() - 1});
    getLine();
    const int n = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
    getLine();
    Dxf_HeaderVariableValue varValue;
    if (n < 10) { // String
        varValue = m_strCache.add(m_str);
    }
    else if (70 <= n && n <= 289) { // Integer
        varValue = stringToInt(m_str);
    }
    else if (40 <= n && n <= 58) { // Double
        varValue = stringToDouble(m_str);
    }
    else if (isXyzCoordGroupCode(n)) { // [X, Y, Z] coords
        DxfCoords coords = {};
        int nCoord = n;
        while (isXyzCoordGroupCode(nCoord)) {
            if (nCoord == 10)
                coords.x = stringToDouble(m_str);
            else if (nCoord == 20)
                coords.y = stringToDouble(m_str);
            else if (nCoord == 30)
                coords.z = stringToDouble(m_str);
            getLine();
            nCoord = stringToInt(m_str, StringToErrorMode::ReturnErrorValue);
            if (!isStringToErrorValue(nCoord))
                getLine();
        }
    }

    if (varValue.index() != 0) { // Not std::monostate
        m_mapHeaderVarValue.insert({ varName, varValue });
        if (varName == "INSUNITS") {
            m_unit = static_cast<DxfUnit>(dxfGetInt(varValue, int(DxfUnit::Unspecified)));
        }
        else if (varName == "MEASUREMENT") {
            m_measurement_inch = dxfGetInt(varValue, 1/*metric*/) == 0;
        }
        else if (varName == "ACADVER") {
            this->resolveAcadVer(dxfGetString(varValue));
        }
        else if (varName == "DWGCODEPAGE") {
            m_codePage = dxfGetString(varValue);
            this->resolveEncoding(m_version);
        }
    }
}

void CDxfRead::read(std::istream& stream)
{
    m_inputStream = &stream;
    m_inputStream->imbue(std::locale::classic());
    auto _ = gsl::finally([&]{ m_inputStream = nullptr; });

    m_strCache.clear();

    m_codePage.clear();

    m_points.clear();
    m_arcs.clear();
    m_circles.clear();
    m_ellipses.clear();
    m_texts.clear();
    m_mtexts.clear();
    m_lines.clear();
    m_lwpolylines.clear();
    m_polylines.clear();
    m_inserts.clear();
    m_3dfaces.clear();
    m_solids.clear();
    m_splines.clear();
    m_blocks.clear();
    m_styles.clear();
    m_layers.clear();
    m_entities.clear();
    m_currentBlock = nullptr;

    m_mapBlock.clear();
    m_mapStyle.clear();
    m_mapLayer.clear();

    m_gcount = 0;
    if (m_fail)
        return;

    getLine();

    ScopedCLocale _c(LC_NUMERIC);
    while (!inputStream().eof()) {
        // Handle header variable
        if (!m_str.empty() && m_str.at(0) == '$')
            this->readHeaderVariable();

        if (m_str == "0") {
            getLine();
            if (m_str == "0")
                getLine(); // Skip again

            auto itHandler = m_mapEntityHandler.find(m_str);
            if (itHandler != m_mapEntityHandler.cend()) {
                const auto& fn = itHandler->second;
                bool okRead = false;
                std::string exceptionMsg;
                try {
                    okRead = fn();
                } catch (const std::runtime_error& err) {
                    exceptionMsg = err.what();
                }

                if (okRead) {
                    continue;
                }
                else {
                    m_fail = false;
                    std::string errMsg = "DXF::DoRead() - Failed to read " + m_str;
                    if (!exceptionMsg.empty())
                        errMsg += "\nError: " + exceptionMsg;

                    this->ReportError(errMsg);
                    if (m_str == "LAYER") // Some objects or tables can have "LAYER" as name...
                        continue;
                    else
                        return;
                }
            }
        }

        getLine();
    }
}

void CDxfRead::ReportError_readInteger(const char* context)
{
    std::string msg;
    if (context) {
        msg += context;
        msg += " - ";
    }

    msg += "Failed to read integer from '";
    msg += m_str;
    msg += "'";
    this->ReportError(msg);
}

std::streamsize CDxfRead::gcount() const
{
    // std::getline() doesn't affect std::istream::gcount
    //return m_ifs.gcount();
    return m_gcount;
}

std::istream& CDxfRead::inputStream()
{
    if (m_inputStream) {
        return *m_inputStream;
    }
    else {
        static std::istringstream istr;
        return istr;
    }
}
