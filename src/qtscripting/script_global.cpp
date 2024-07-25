/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_global.h"

#include "../qtcommon/qstring_conv.h"
#include "script_application.h"

#include <QtCore/QMessageLogger>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <GeomAbs_BSplKnotDistribution.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomAbs_Shape.hxx>
#include <GeomAbs_SurfaceType.hxx>

namespace Mayo {

namespace {

template<typename Enum> using ScriptEnumValue = std::pair<std::string_view, Enum>;
template<typename Enum> struct ScriptEnumBinding {};

template<> struct ScriptEnumBinding<GeomAbs_BSplKnotDistribution> {
    static constexpr std::string_view strName = "GeomBSplineKnotDistribution";
    static constexpr ScriptEnumValue<GeomAbs_BSplKnotDistribution> values[] = {
        { "NonUniform", GeomAbs_NonUniform },
        { "Uniform", GeomAbs_Uniform },
        { "QuasiUniform", GeomAbs_QuasiUniform },
        { "PiecewiseBezier", GeomAbs_PiecewiseBezier }
    };
};

template<> struct ScriptEnumBinding<GeomAbs_Shape> {
    static constexpr std::string_view strName = "GeomContinuity";
    static constexpr ScriptEnumValue<GeomAbs_Shape> values[] = {
        { "C0", GeomAbs_C0 },
        { "G1", GeomAbs_G1 },
        { "C1", GeomAbs_C1 },
        { "G2", GeomAbs_G2 },
        { "C2", GeomAbs_C2 },
        { "C3", GeomAbs_C3 },
        { "CN", GeomAbs_CN }
    };
};

template<> struct ScriptEnumBinding<GeomAbs_CurveType> {
    static constexpr std::string_view strName = "GeomCurveType";
    static constexpr ScriptEnumValue<GeomAbs_CurveType> values[] = {
        { "Line", GeomAbs_Line },
        { "Circle", GeomAbs_Circle },
        { "Ellipse", GeomAbs_Ellipse },
        { "Hyperbola", GeomAbs_Hyperbola },
        { "Parabola", GeomAbs_Parabola },
        { "Bezier", GeomAbs_BezierCurve },
        { "BSpline", GeomAbs_BSplineCurve },
        { "Offset", GeomAbs_OffsetCurve },
        { "Other", GeomAbs_OtherCurve }
    };
};

template<> struct ScriptEnumBinding<GeomAbs_SurfaceType> {
    static constexpr std::string_view strName = "GeomSurfaceType";
    static constexpr ScriptEnumValue<GeomAbs_SurfaceType> values[] = {
        { "Plane", GeomAbs_Plane },
        { "Cylinder", GeomAbs_Cylinder },
        { "Cone", GeomAbs_Cone },
        { "Sphere", GeomAbs_Sphere },
        { "Torus", GeomAbs_Torus },
        { "Bezier", GeomAbs_BezierSurface },
        { "BSpline", GeomAbs_BSplineSurface },
        { "SurfaceOfRevolution", GeomAbs_SurfaceOfRevolution },
        { "SurfaceOfExtrusion", GeomAbs_SurfaceOfExtrusion },
        { "Offset", GeomAbs_OffsetSurface },
        { "Other", GeomAbs_OtherSurface }
    };
};

template<> struct ScriptEnumBinding<TopAbs_ShapeEnum> {
    static constexpr std::string_view strName = "ShapeType";
    static constexpr ScriptEnumValue<TopAbs_ShapeEnum> values[] = {
        { "Compound", TopAbs_COMPOUND },
        { "CompoundSolid", TopAbs_COMPSOLID },
        { "Solid", TopAbs_SOLID },
        { "Shell", TopAbs_SHELL },
        { "Face", TopAbs_FACE },
        { "Wire", TopAbs_WIRE },
        { "Edge", TopAbs_EDGE },
        { "Vertex", TopAbs_VERTEX }
    };
};

template<> struct ScriptEnumBinding<TopAbs_Orientation> {
    static constexpr std::string_view strName = "ShapeOrientation";
    static constexpr ScriptEnumValue<TopAbs_Orientation> values[] = {
        { "Forward", TopAbs_FORWARD },
        { "Reversed", TopAbs_REVERSED },
        { "Internal", TopAbs_INTERNAL },
        { "External", TopAbs_EXTERNAL }
    };
};

template<typename Enum>
void addScriptEnum(QJSEngine* jsEngine)
{
    QJSValue scriptEnum = jsEngine->newObject();
    for (const auto& [strEnumValueName, enumValue] : ScriptEnumBinding<Enum>::values)
        scriptEnum.setProperty(to_QString(strEnumValueName), enumValue);

    QString strEnumPropName = to_QString(ScriptEnumBinding<Enum>::strName);
    jsEngine->globalObject().setProperty(strEnumPropName, scriptEnum);
}

} // namespace

QJSEngine* createScriptEngine(const ApplicationPtr& app, QObject* parent)
{
    auto jsEngine = new QJSEngine(parent);
    jsEngine->installExtensions(QJSEngine::ConsoleExtension);

    auto scriptApp = new ScriptApplication(app, jsEngine);
    QJSValue jsApp = jsEngine->newQObject(scriptApp);
    jsEngine->globalObject().setProperty("application", jsApp);

    addScriptEnum<GeomAbs_BSplKnotDistribution>(jsEngine);
    addScriptEnum<GeomAbs_CurveType>(jsEngine);
    addScriptEnum<GeomAbs_Shape>(jsEngine);
    addScriptEnum<GeomAbs_SurfaceType>(jsEngine);
    addScriptEnum<TopAbs_ShapeEnum>(jsEngine);
    addScriptEnum<TopAbs_Orientation>(jsEngine);

    return jsEngine;
}

void logScriptError(const QJSValue& jsVal, const char* functionName)
{
    if (jsVal.isError()) {
        const QByteArray name = jsVal.property("name").toString().toUtf8();
        const QByteArray message = jsVal.property("message").toString().toUtf8();
        const QByteArray fileName = jsVal.property("fileName").toString().toUtf8();
        const int lineNumber = jsVal.property("lineNumber").toInt();
        const QMessageLogger msgLogger(fileName.constData(), lineNumber, functionName, "js");
        msgLogger.critical("%s: %s", name.constData(), message.constData());
    }
}

} // namespace Mayo
