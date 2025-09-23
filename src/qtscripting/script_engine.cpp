/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_engine.h"
#include "script_application.h"
#include "script_document.h"
#include "script_mayo.h"
#include "script_typedefs.h"

#include "../base/io_system.h"
#include "../base/io_parameters_provider.h"
#include "../base/property_enumeration.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/log_message_handler.h"
#include "../qtcommon/qtcore_utils.h"
#include "../qtcommon/qstring_conv.h"

#include <QtCore/QFile>
#include <QtCore/QMessageLogger>
#include <QtQml/QJSEngine>

#include <GeomAbs_BSplKnotDistribution.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomAbs_Shape.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopAbs_ShapeEnum.hxx>

namespace Mayo {

namespace {

//
// Helper functions dedicated to C++/JS enumeration mapping
//

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
        { "Vertex", TopAbs_VERTEX },
        { "Shape", TopAbs_SHAPE }
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

//
// Misc helper functions
//

QString scriptProgram(const QString& strFilepath)
{
    QFile file(strFilepath);
    if (file.open(QIODevice::ReadOnly))
        return file.readAll();

    return {};
}

MessageType toMayoMessageType(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return MessageType::Trace;
    case QtInfoMsg:
        return MessageType::Info;
    case QtWarningMsg:
        return MessageType::Warning;
    case QtCriticalMsg:
    //case QtSystemMsg:
    case QtFatalMsg:
        return MessageType::Error;
    }

    return MessageType::Trace;
}

} // namespace

ScriptEngine::ScriptEngine(const ApplicationPtr& mainApp, const ScriptEnvironment& env)
    : m_mainApp(mainApp),
      m_scriptEnv(env)
{
    m_taskMgr.signalStarted.connectSlot(&ScriptEngine::onTaskStarted, this);
    m_taskMgr.signalEnded.connectSlot(&ScriptEngine::onTaskEnded, this);
}

void ScriptEngine::setScriptFilePath(const FilePath& filePath)
{
    m_scriptFilePath = filepathCanonical(filePath).make_preferred();
}

void ScriptEngine::runEvaluate()
{
    if (m_isEvaluateRunning)
        return;

    m_scriptExecTaskId = m_taskMgr.newTask([=](TaskProgress*) {
        // Override the "console output" handler of LogMessageHandler, first keep the current handler
        // so it can be restored before exiting
        auto& logMsgHandler = LogMessageHandler::instance();
        auto onEntryJsConsoleOutputHandler = logMsgHandler.jsConsoleOutputHandler();
        auto _ = gsl::finally([&]{
            logMsgHandler.setJsConsoleOutputHandler(onEntryJsConsoleOutputHandler);
        });
        logMsgHandler.setJsConsoleOutputHandler(
            [=](QtMsgType type, const QMessageLogContext& context, const QString& text) {
                Message msg;
                msg.type = toMayoMessageType(type);
                msg.text = to_stdString(text);
                msg.contextFile = std::string{context.file ? context.file : ""};
                msg.contextLine = context.line;
                this->signalMessage.send(msg);
            }
        );

        // Evaluate script program
        m_jsEngine = new QJSEngine;
        ScriptEngine::init(m_jsEngine, m_mainApp, m_scriptEnv);
        const QString strScriptFilePath = filepathTo<QString>(m_scriptFilePath);
        auto jsVal = m_jsEngine->evaluate(scriptProgram(strScriptFilePath), strScriptFilePath);
        ScriptEngine::logError(jsVal);
    });
    m_taskMgr.run(m_scriptExecTaskId);
}

void ScriptEngine::stopEvaluate()
{
    if (m_jsEngine) {
        m_wasEvaluateStopped = true;
        m_jsEngine->setInterrupted(true);
    }
}

void ScriptEngine::runOrStopEvaluate()
{
    if (m_isEvaluateRunning)
        this->stopEvaluate();
    else
        this->runEvaluate();
}

bool ScriptEngine::isEvaluateRunning() const
{
    return m_isEvaluateRunning;
}

bool ScriptEngine::waitForEvaluateEnd(int msecs)
{
    return m_taskMgr.waitForDone(m_scriptExecTaskId, msecs);
}

void ScriptEngine::logError(const QJSValue& jsVal, const char* functionName)
{
    if (jsVal.isError()) {
        //const QByteArray name = jsVal.property("name").toString().toUtf8();
        const QByteArray message = jsVal.property("message").toString().toUtf8();
        const QByteArray fileName = jsVal.property("fileName").toString().toUtf8();
        const int lineNumber = jsVal.property("lineNumber").toInt();
        const QMessageLogger msgLogger(fileName.constData(), lineNumber, functionName, "js");
        //msgLogger.critical("%s: %s", name.constData(), message.constData());
        msgLogger.critical(message.constData());
    }
}

void ScriptEngine::logError(QJSEngine* jsEngine, std::string_view message, const char* functionName)
{
    if (jsEngine) {
        auto jsError = jsEngine->newErrorObject(QJSValue::GenericError, to_QString(message));
        ScriptEngine::logError(jsError, functionName);
    }
}

// Configures the JS engine so it can support Mayo Scripting API
//
// * Installs the required JS extensions(eg Console)
//
// * Registers a global 'Mayo' JS object bound to 'app' parameter. This implies JS scripts can
//   access the already loaded documents in Mayo
//
// * Registers Mayo Scripting enumerations as plain objects for cleaner syntax in JS scripts
//   Example:
//       GeomCurveType {
//           Line: intLiteral,
//           Circle: intLiteral,
//           Ellipse: intLiteral,
//           ...
//        };
//   To be used in JS code as GeomCurveType.Circle instead of error-prone int/string literal
//   This also includes enums declared for any IO reader/writer provided by the IO::System object
//   of input ScriptEnvironment
void ScriptEngine::init(QJSEngine* jsEngine, const ApplicationPtr& app, const ScriptEnvironment& env)
{
    if (!jsEngine)
        return;

    jsEngine->installExtensions(QJSEngine::ConsoleExtension);

    auto scriptMayo = new ScriptMayo(env, jsEngine);
    scriptMayo->setMainApplication(app);
    QJSValue jsMayoObject = jsEngine->newQObject(scriptMayo);
    jsEngine->globalObject().setProperty("Mayo", jsMayoObject);

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
        qRegisterMetaType<Ptr_ScriptApplication>("Ptr_ScriptApplication");
        qRegisterMetaType<Ptr_ScriptDocument>("Ptr_ScriptDocument");
        qRegisterMetaType<QVariant_ScriptTreeNode>("QVariant_ScriptTreeNode");
        qRegisterMetaType<QVariant_Coords3D>("QVariant_Coords3D");
        qRegisterMetaType<ScriptGeomBSplineKnotDistribution>("ScriptGeomBSplineKnotDistribution");
        qRegisterMetaType<ScriptGeomContinuity>("ScriptGeomContinuity");
        qRegisterMetaType<ScriptGeomCurveType>("ScriptGeomCurveType");
        qRegisterMetaType<ScriptGeomSurfaceType>("ScriptGeomSurfaceType");
        qRegisterMetaType<ScriptShapeOrientation>("ScriptShapeOrientation");
        qRegisterMetaType<ScriptShapeType>("ScriptShapeType");
        qRegisterMetaType<TaskId>("TaskId");
        qRegisterMetaType<TreeNodeId>("TreeNodeId");
        qRegisterMetaType<QJSValue_JsonObject>("QJSValue_JsonObject");
        qRegisterMetaType<QJSValueList_ArrayOfTaskId>("QJSValueList_ArrayOfTaskId");
        qRegisterMetaType<QJSValue_MayoTraverseModelTreeCallback>("QJSValue_MayoTraverseModelTreeCallback");
        qRegisterMetaType<QJSValue_MayoTraverseShapeCallback>("QJSValue_MayoTraverseShapeCallback");
        qRegisterMetaType<QJSValue_ScriptDocument>("QJSValue_ScriptDocument");
        qRegisterMetaType<QJSValue_ScriptShape>("QJSValue_ScriptShape");
        metaTypesRegistered = true;
    }
}

void ScriptEngine::onTaskStarted(TaskId taskId)
{
    if (m_scriptExecTaskId != taskId)
        return;

    m_isEvaluateRunning = true;
    m_wasEvaluateStopped = false;
    this->signalEvaluateStarted.send();
}

void ScriptEngine::onTaskEnded(TaskId taskId)
{
    if (m_scriptExecTaskId != taskId)
        return;

    m_isEvaluateRunning = false;
    if (!m_wasEvaluateStopped)
        this->signalEvaluateEnded.send(EndReason::Finished);
    else
        this->signalEvaluateEnded.send(EndReason::Stopped);

    m_jsEngine->setInterrupted(false);
    delete m_jsEngine;
    m_jsEngine = nullptr;
}

} // namespace Mayo
