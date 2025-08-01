/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_document.h"

#include "../base/brep_utils.h"
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
#include "script_shape.h"
#include "script_tree_node.h"

#include <TDF_Tool.hxx>

#include <QtCore/QtDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValueIterator>

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

unsigned ScriptDocument::entityTreeNodeId(int index) const
{
    return m_doc ? m_doc->entityTreeNodeId(index) : 0;
}

void ScriptDocument::traverseModelTree(QJSValue fn)
{
    if (!fn.isCallable())
        return;

    const Tree<TDF_Label>& modelTree = m_doc->modelTree();
    traverseTree(modelTree, [&](TreeNodeId nodeId) {
#if 0
        const TreeNodeId parentNodeId = modelTree.nodeParent(nodeId);
        if (parentNodeId != 0) {
            const TDF_Label& parentNodeLabel = modelTree.nodeData(parentNodeId);
            if (XCaf::isShapeReference(parentNodeLabel))
                return; // Skip: tree node is a product(or "referred" shape)
        }
#endif
        auto jsVal = fn.call({ QJSValue{nodeId} });
        logScriptError(jsVal, "traverseModelTree()");
    });
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

QVariant_ScriptTreeNode ScriptDocument::treeNode(unsigned int treeNodeId) const
{
    return QVariant::fromValue(ScriptTreeNode(m_doc, treeNodeId));
}

void ScriptDocument::traverseShape(QJSValue shape, unsigned shapeTypeFilter, QJSValue fn)
{
    if (!fn.isCallable())
        return;

    if (!m_jsApp || m_jsApp->jsEngine())
        return;

    const auto shapeTypeEnum = static_cast<TopAbs_ShapeEnum>(shapeTypeFilter);
    const auto scriptShape = m_jsApp->jsEngine()->fromScriptValue<ScriptShape>(shape);
    BRepUtils::forEachSubShape(scriptShape.shape(), shapeTypeEnum, [&](const TopoDS_Shape& subShape) {
        auto jsSubShape = m_jsApp->jsEngine()->toScriptValue(ScriptShape(subShape));
        auto jsVal = fn.call({ jsSubShape });
        logScriptError(jsVal, "traverseShape()");
    });
}

static void printJsonValue(const QJsonValue& jsonValue, int indent = 0)
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

static QJsonValue findJsonValueFromName(const QJsonValue& jsonValue, std::string_view strName)
{
    if (!jsonValue.isObject())
        return {};

    const QString qstrName = to_QString(strName);
    const QJsonObject jsonObj = jsonValue.toObject();
    for (const QString& key : jsonObj.keys()) {
        if (key == qstrName)
            return jsonObj.value(key);
    }

    return {};
}

void ioUpdateParametersFromJson(
        const QJsonObject& jsonParams,
        PropertyGroup* parameters,
        const PropertyValueConversion& propValueConverter
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
            else
                qWarning() << "ioUpdateParametersFromJson() Call to PropertyValueConversion::fromVariant() failed for param" << to_QString(paramName);
        }
        else {
            qWarning() << "ioUpdateParametersFromJson() Did not find property for param" << to_QString(paramName);
        }

    }
}

namespace {

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

quint32 ScriptDocument::asyncImportFile(QString strFilepath, QJSValue jsOptions, QJSValue fnCallbacks)
{
    // Make it a pointer so it can be captured by value in lambda functions
    const ScriptEnvironment* env = &m_jsApp->environment();
    if (!env->ioSystem)
        return false;

    const QJsonValue jsonOptions = QJsonValue::fromVariant(jsOptions.toVariant());
    printJsonValue(jsonOptions);

    const IO::Format format = env->ioSystem->probeFormat(filepathFrom(strFilepath));
    auto messengerPtr = std::make_unique<MessengerBySignal>();
    auto messenger = messengerPtr.get();
    TaskManager& taskMgr = m_jsApp->taskManager();
    auto importTaskId = taskMgr.newTask([=](TaskProgress* progress) {
        ImplParametersProvider jsParamsProvider;
        auto readerParams = this->createReaderParametersFromJson(jsonOptions, format);
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

    ScriptApplication::TaskCallbacks callbacks;
    for (QJSValueIterator it(fnCallbacks); it.hasNext();) {
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

    m_jsApp->registerTask(importTaskId, callbacks, std::move(messengerPtr));
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

std::unique_ptr<PropertyGroup> ScriptDocument::createReaderParametersFromJson(const QJsonValue& jsonOptions, IO::Format format) const
{
    const ScriptEnvironment& env = m_jsApp->environment();

    const QJsonValue jsonFormatParams = findJsonValueFromName(jsonOptions, IO::formatIdentifier(format));
    if (!jsonFormatParams.isObject())
        return {};

    const auto factoryReader = env.ioSystem->findFactoryReader(format);
    if (!factoryReader)
        return {};

    auto ptrParams = factoryReader->createProperties(format, nullptr);
    const PropertyGroup* ptrDefaultParams = env.ioParametersProvider->findReaderParameters(format);
    const auto& propValueConverter = ScriptEnvironment::getPropertyValueConverter(env);
    PropertyValueConversion::copyValues(ptrParams.get(), *ptrDefaultParams, propValueConverter);
    ioUpdateParametersFromJson(jsonFormatParams.toObject(), ptrParams.get(), propValueConverter);
    return ptrParams;
}

} // namespace Mayo
