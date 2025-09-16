/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_document.h"

#include "../base/document.h"
#include "../base/io_parameters_provider.h"
#include "../base/io_system.h"
#include "../base/property_value_conversion.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "../qtcommon/qtcore_utils.h"
#if 0
#  include "../app/qtgui_utils.h"
#endif
#include "script_application.h"
#include "script_global.h"
#include "script_mayo.h"
#include "script_shape.h"
#include "script_tree_node.h"

#include <TDF_Tool.hxx>

#include <QtCore/QtDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValueIterator>

#include <fmt/format.h>

namespace Mayo {

int ScriptDocument::id() const
{
    return m_doc ? m_doc->identifier() : -1;
}

QString ScriptDocument::name() const
{
    return m_doc ? to_QString(m_doc->name()) : QString{};
}

void ScriptDocument::setName(const QString& str)
{
    if (m_doc)
        m_doc->setName(to_stdString(str));
}

QString ScriptDocument::filePath() const
{
    return m_doc ? filepathTo<QString>(m_doc->filePath()) : QString{};
}

void ScriptDocument::setFilePath(const QString& str)
{
    if (m_doc)
        m_doc->setFilePath(filepathFrom(str));
}

int ScriptDocument::entityCount() const
{
    return m_doc ? m_doc->entityCount() : 0;
}

//! \brief Returns identifier of the tree node corresponding to entity(root node) at `index`
//! \pre `0 ≤ index < entityCount`
//! \returns 0 if `index` is out of bounds
TreeNodeId ScriptDocument::entityTreeNodeId(int index) const
{
    return m_doc ? m_doc->entityTreeNodeId(index) : 0;
}

#if 0
QColor ScriptDocument::tagShapeColor(const QString& tag) const
{
    if (!m_doc)
        return {};

    TDF_Label label;
    TDF_Tool::Label(m_doc->GetData(), to_OccAsciiString(tag), label);
    if (label.IsNull())
        return {};

    return QtGuiUtils::toQColor(m_doc->xcaf().shapeColor(label));
}
#endif

//! \brief Returns the TreeNode object in the model tree corresponding to identifier `treeNodeId`
//!
//! This function is often useful in conjunction with traverseModelTree()
QVariant_ScriptTreeNode ScriptDocument::treeNode(TreeNodeId treeNodeId) const
{
    return QVariant::fromValue(ScriptTreeNode(m_doc, treeNodeId));
}

namespace {

void printJsonValue(const QJsonValue& jsonValue, int indent = 0)
{
    if (jsonValue.isObject()) {
        const QJsonObject jsonObj = jsonValue.toObject();
        for (const QString& key : jsonObj.keys()) {
            if (jsonObj.value(key).isObject()) {
                qDebug().noquote() << key;
                printJsonValue(jsonObj.value(key), indent + 1);
            }
            else {
                qDebug().noquote() << QString(indent * 4, ' ') << key << ":" << jsonObj.value(key).toVariant().toString();
            }
        }
    }
    else {
        qDebug().noquote() << QString(indent * 4, ' ') << jsonValue.toVariant().toString();
    }
}

QJsonObject findJsonFormatParameters(const QJsonValue& jsonValue, IO::Format format)
{
    if (!jsonValue.isObject())
        return {};

    const QString formatName = to_QString(IO::formatIdentifier(format));
    const QJsonObject jsonObj = jsonValue.toObject();
    for (const QString& key : jsonObj.keys()) {
        if (QString::compare(key, formatName, Qt::CaseInsensitive) == 0)
            return jsonObj.value(key).toObject();
    }

    return {};
}

// Various errors that may be reported in ioUpdateParametersFromJson()
enum class IOUpdateParametersFromJsonError {
    ParameterNotFound, ValueConversionFailed
};

// Type alias for the error callback invoked by ioUpdateParametersFromJson()
using IOUpdateParametersFromJsonErrorCallback = std::function<void(IOUpdateParametersFromJsonError, std::string_view, std::string_view)>;

// Update the I/O properties in 'parameters' from the JSON object 'jsonParams'
// The input JSON object is expected to be flat(only simple values, no inner objects), and its keys
// must match the property names
// The provided PropertyValueConversion object is used to copy the JSON values to the corresponding
// I/O parameters
// In any case of error the function errorCallback is invoked with information about the parameter
void ioUpdateParametersFromJson(
        const QJsonObject& jsonParams,
        PropertyGroup* parameters,
        const PropertyValueConversion& propValueConverter,
        const IOUpdateParametersFromJsonErrorCallback& errorCallback = nullptr
    )
{
    if (!parameters)
        return;

    for (const QString& key : jsonParams.keys()) {
        const std::string paramName = to_stdString(key);
        auto itPropParam = std::find_if(
            parameters->properties().begin(), parameters->properties().end(),
            [=](const Property* prop) { return prop->name().key == paramName; }
        );
        if (itPropParam != parameters->properties().end()) {
            auto value = QtCoreUtils::toPropertyValueConversionVariant(jsonParams.value(key).toVariant());
            const bool ok = propValueConverter.fromVariant(*itPropParam, value);
            if (ok)
                qDebug() << "ioUpdateParametersFromJson() Override value of param" << to_QString(paramName) << "with" << to_QString(value.toString());

            if (!ok && errorCallback)
                errorCallback(IOUpdateParametersFromJsonError::ValueConversionFailed, paramName, value.toString());
        }
        else {
            if (errorCallback)
                errorCallback(IOUpdateParametersFromJsonError::ParameterNotFound, paramName, std::string{});
        }
    }
}

// Simple ParametersProvider that stores the I/O parameters for further access
// TODO Move in Base module?
class ImplParametersProvider : public IO::ParametersProvider {
public:
    using PropertyGroupPtr = std::unique_ptr<PropertyGroup>;

    bool hasReaderParameters() const { return !m_vecReaderParams.empty(); }
    bool hasWriterParameters() const { return !m_vecWriterParams.empty(); }

    void addReaderParameters(IO::Format format, PropertyGroupPtr ptr)
    {
        if (ptr)
            m_vecReaderParams.push_back({format, std::move(ptr)});
    }

    void addWriterParameters(IO::Format format, PropertyGroupPtr ptr)
    {
        if (ptr)
            m_vecWriterParams.push_back({format, std::move(ptr)});
    }

    const PropertyGroup* findReaderParameters(IO::Format format) const override
    {
        return ImplParametersProvider::findParameters(format, m_vecReaderParams);
    }

    const PropertyGroup* findWriterParameters(IO::Format format) const override
    {
        return ImplParametersProvider::findParameters(format, m_vecWriterParams);
    }

private:
    struct Parameters {
        IO::Format format;
        PropertyGroupPtr ptr;
    };

    static const PropertyGroup* findParameters(IO::Format format, Span<const Parameters> spanParams)
    {
        auto it = std::find_if(
            spanParams.begin(), spanParams.end(),
            [=](const Parameters& params) { return params.format == format; }
        );
        return it != spanParams.end() ? it->ptr.get() : nullptr;
    }

    std::vector<Parameters> m_vecReaderParams;
    std::vector<Parameters> m_vecWriterParams;
};

} // namespace

/*!
  \brief Runs an asynchronous task to import file at location `strFilepath` into the Document object

  The return value is the import task identifier . This identifier can be used with
  \ref Mayo::waitForDone(TaskId, int) "Mayo.waitForDone(TaskId)" to wait for completion of import
  task.

  \param strFilepath Location of the file to be imported. Prefer absolute file path
  \param jsOptions Options for the import operation. This parameter is specified as a JSON object
  and must use special members.<br>
  Root members take the name of the supported file formats, eg STEP, IGES, DXF, GLTF, ...<br>
  Each file format has its own parameters that can be specified, eg:
  \code{.js}
  {
      STEP: {
          readSubShapesNames: true,
          productContext: STEP.ProductContext.Both
      },
      DXF: {
          importAnnotations: true,
          groupLayers: true
      }
  }
  \endcode
  Although only one set of paremters will be used(the format matching the input file), parameters
  for multiple file formats can be specified(other formats will just be ignored).<br>
  This allows to define specific parameters in a single object(eg one JS constant) and pass it to
  this function.<br>
  Note that these parameters are optional. By default the parameter values are taken from mayo user
  settings
  \param jsCallbacks Callbacks for the import operation. This parameter is specified as a JSON object
*/
TaskId ScriptDocument::asyncImportFile(
        QString strFilepath, QJSValue_JsonObject jsOptions, QJSValue_JsonObject jsCallbacks
    )
{
    // Make it a pointer so it can be captured by value in lambda functions
    const ScriptEnvironment* env = &m_jsApp->mayoObject()->environment();
    if (!env->ioSystem)
        return false;

    const QJsonValue jsonOptions = QJsonValue::fromVariant(jsOptions.toVariant());
    printJsonValue(jsonOptions);

    const IO::Format format = env->ioSystem->probeFormat(filepathFrom(strFilepath));
    auto messengerPtr = std::make_unique<MessengerBySignal>();
    auto messenger = messengerPtr.get();
    TaskManager& taskMgr = m_jsApp->mayoObject()->taskManager();
    auto importTaskId = taskMgr.newTask([=](TaskProgress* progress) {
        ImplParametersProvider jsParamsProvider;
        auto readerParams = this->createReaderParametersFromJson(jsonOptions, format, [=](std::string_view err) {
            logScriptError(m_jsApp->mayoObject()->jsEngine(), err, "Document.asyncImportFile()");
        });
        jsParamsProvider.addReaderParameters(format, std::move(readerParams));
        env->ioSystem->importInDocument()
            .targetDocument(this->baseDocument())
            .withFilepath(filepathFrom(strFilepath))
            .withParametersProvider(
                jsParamsProvider.hasReaderParameters() ? &jsParamsProvider : env->ioParametersProvider
             )
            .withEntityPostProcess(env->ioEntityImportPostProcess)
            .withEntityPostProcessRequiredIf(&IO::formatProvidesBRep)
            .withEntityPostProcessInfoProgress(20, "Mesh BRep shapes")
            .withTaskProgress(progress)
            .withMessenger(messenger)
            .execute()
        ;
    });

    ScriptMayo::TaskCallbacks callbacks;
    for (QJSValueIterator it(jsCallbacks); it.hasNext();) {
        it.next();
        if (it.name() == "onStarted") {
            callbacks.onStarted = it.value();
        }
        else if (it.name() == "onProgress") {
            callbacks.onProgress = it.value();
        }
        else if (it.name() == "onEnded") {
            callbacks.onEnded = it.value();
        }
        else if (it.name() == "onWarning") {
            callbacks.onWarning = it.value();
        }
        else if (it.name() == "onError") {
            callbacks.onError = it.value();
        }
    }

    m_jsApp->mayoObject()->registerTask(importTaskId, callbacks, std::move(messengerPtr));
    taskMgr.run(importTaskId);
    return importTaskId;
}

ScriptDocument::ScriptDocument(const DocumentPtr& doc, ScriptApplication* jsApp)
    : QObject(jsApp),
      m_jsApp(jsApp),
      m_doc(doc)
{
    m_sigConns
        << doc->signalNameChanged.connectSlot([=](const std::string&) { emit this->nameChanged(); })
        << doc->signalFilePathChanged.connectSlot([=](const FilePath&) { emit this->filePathChanged(); })
        << doc->signalEntityAdded.connectSlot([=](TreeNodeId) { emit this->entityCountChanged(); })
        << doc->signalEntityAboutToBeDestroyed.connectSlot([=](TreeNodeId) { emit this->entityCountChanged(); })
    ;
}

std::unique_ptr<PropertyGroup> ScriptDocument::createReaderParametersFromJson(
        const QJsonValue& jsonOptions,
        IO::Format format,
        const std::function<void(std::string_view)>& errorCallback
    ) const
{
    const QJsonObject jsonFormatParams = findJsonFormatParameters(jsonOptions, format);
    if (jsonFormatParams.empty())
        return {};

    const ScriptEnvironment& env = m_jsApp->mayoObject()->environment();
    const auto factoryReader = env.ioSystem->findFactoryReader(format);
    if (!factoryReader)
        return {};

    auto ptrParams = factoryReader->createProperties(format, nullptr);
    const PropertyGroup* ptrDefaultParams = env.ioParametersProvider->findReaderParameters(format);
    const auto& propValueConverter = ScriptEnvironment::getPropertyValueConverter(env);
    PropertyValueConversion::copyValues(ptrParams.get(), *ptrDefaultParams, propValueConverter);

    IOUpdateParametersFromJsonErrorCallback ioUpdateErrorCallback =
        [&](IOUpdateParametersFromJsonError err, std::string_view paramName, std::string_view paramValue) {
            auto strFormat = IO::formatIdentifier(format);
            switch (err) {
            case IOUpdateParametersFromJsonError::ParameterNotFound:
                errorCallback(fmt::format(textIdTr("Unknown parameter [name={}.{}]"), strFormat, paramName));
                break;
            case IOUpdateParametersFromJsonError::ValueConversionFailed:
                errorCallback(
                    fmt::format(textIdTr("Conversion of parameter value failed [name={}.{}, value={}]"),
                                strFormat, paramName, paramValue)
                );
                break;
            }
        };
    ioUpdateErrorCallback = errorCallback ? ioUpdateErrorCallback : nullptr;
    ioUpdateParametersFromJson(jsonFormatParams, ptrParams.get(), propValueConverter, ioUpdateErrorCallback);
    return ptrParams;
}

} // namespace Mayo
