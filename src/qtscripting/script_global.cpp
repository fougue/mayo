/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_global.h"

#include "../base/io_system.h"
#include "../base/io_parameters_provider.h"
#include "../base/property_enumeration.h"
#include "../base/property_value_conversion.h"
#include "../qtcommon/qstring_conv.h"
#include "script_application.h"
#include "script_geom_curve.h"
#include "script_geom_surface.h"
#include "script_shape.h"
#include "script_tree_node.h"

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

QJSValue createScriptEnum(const Property* property, QJSEngine* jsEngine)
{
    if (!property || property->dynTypeName() != PropertyEnumeration::TypeName)
        return {};

    const auto propEnum = dynamic_cast<const PropertyEnumeration*>(property);

    // Check if the enumeration can be declared as a JS object
    bool enumKeysMayContainSpaces = false;
    for (const Enumeration::Item& enumItem : propEnum->enumeration().items()) {
        if (enumItem.name.key.find(' ') != std::string::npos) {
            enumKeysMayContainSpaces = true;
            break; // Halt
        }
    }

    if (enumKeysMayContainSpaces)
        return {};

    // Create JS object to declare the enumeration values
    QJSValue objectEnum = jsEngine->newObject();
    for (const Enumeration::Item& enumItem : propEnum->enumeration().items())
        objectEnum.setProperty(to_QString(enumItem.name.key), enumItem.value);

    return objectEnum;
}

std::string_view getEnumerationTypeName(const Property* property)
{
    if (!property || property->dynTypeName() != PropertyEnumeration::TypeName)
        return {};

    const auto propEnum = dynamic_cast<const PropertyEnumeration*>(property);
    return propEnum->enumeration().name();
}

} // namespace

const PropertyValueConversion& ScriptEnvironment::getPropertyValueConverter(const ScriptEnvironment& env)
{
    static const PropertyValueConversion defaultPropValueConverter;
    return env.propertyValueConverter ? *env.propertyValueConverter : defaultPropValueConverter;
}

void initScriptEngine(QJSEngine* jsEngine, const ApplicationPtr& app, const ScriptEnvironment& env)
{
    if (!jsEngine)
        return;

    jsEngine->installExtensions(QJSEngine::ConsoleExtension);

    auto scriptApp = new ScriptApplication(app, env, jsEngine);
    QJSValue jsApp = jsEngine->newQObject(scriptApp);
    jsEngine->globalObject().setProperty("application", jsApp);

    addScriptEnum<GeomAbs_BSplKnotDistribution>(jsEngine);
    addScriptEnum<GeomAbs_CurveType>(jsEngine);
    addScriptEnum<GeomAbs_Shape>(jsEngine);
    addScriptEnum<GeomAbs_SurfaceType>(jsEngine);
    addScriptEnum<TopAbs_ShapeEnum>(jsEngine);
    addScriptEnum<TopAbs_Orientation>(jsEngine);

    if (env.ioSystem && env.ioParametersProvider) {
        std::unordered_map<IO::Format, QJSValue> mapFormatEnumObjects;
        auto fnAddScriptEnum = [&](IO::Format format, std::string_view enumTypeName, const QJSValue& jsEnum) {
            if (!jsEnum.isNull()) {
                if (mapFormatEnumObjects.find(format) == mapFormatEnumObjects.cend())
                    mapFormatEnumObjects.insert({format, jsEngine->newObject()});

                QJSValue& objectFormatEnum = mapFormatEnumObjects.find(format)->second;
                objectFormatEnum.setProperty(to_QString(enumTypeName), jsEnum);
            }
        };

        for (IO::Format format : env.ioSystem->readerFormats()) {
            const PropertyGroup* params = env.ioParametersProvider->findReaderParameters(format);
            if (params) {
                for (const Property* prop : params->properties())
                    fnAddScriptEnum(format, getEnumerationTypeName(prop), createScriptEnum(prop, jsEngine));
            }
        }

        for (IO::Format format : env.ioSystem->writerFormats()) {
            const PropertyGroup* params = env.ioParametersProvider->findWriterParameters(format);
            if (params) {
                for (const Property* prop : params->properties())
                    fnAddScriptEnum(format, getEnumerationTypeName(prop), createScriptEnum(prop, jsEngine));
            }
        }

        for (const auto& [format, jsValue] : mapFormatEnumObjects) {
            const QString strFormatName = to_QString(IO::formatIdentifier(format));
            jsEngine->globalObject().setProperty(strFormatName, jsValue);
        }
    }

    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        // NOTE
        //     In Qt5 qRegisterMetaType() must be called for all Script enumeration types and Script
        //     type aliases appearing in functions marked with Q_INVOKABLE
        //     Call to Q_DECLARE_METATYPE() seems to be not needed
        qRegisterMetaType<QObjectPtr_ScriptDocument>("QObjectPtr_ScriptDocument");
        qRegisterMetaType<QVariant_ScriptTreeNode>("QVariant_ScriptTreeNode");
        qRegisterMetaType<QVariant_Coords3D>("QVariant_Coords3D");
        qRegisterMetaType<ScriptGeomBSplineKnotDistribution>("ScriptGeomBSplineKnotDistribution");
        qRegisterMetaType<ScriptGeomContinuity>("ScriptGeomContinuity");
        qRegisterMetaType<ScriptGeomCurveType>("ScriptGeomCurveType");
        qRegisterMetaType<ScriptGeomSurfaceType>("ScriptGeomSurfaceType");
        qRegisterMetaType<ScriptShapeOrientation>("ScriptShapeOrientation");
        qRegisterMetaType<ScriptShapeType>("ScriptShapeType");
        metaTypesRegistered = true;
    }
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
