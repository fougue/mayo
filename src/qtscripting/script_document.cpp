/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_document.h"

#include "../base/brep_utils.h"
#include "../base/document.h"
#include "../base/io_system.h"
#include "../base/task_manager.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#if 0
#  include "../app/qtgui_utils.h"
#endif
#include "script_application.h"
#include "script_global.h"
#include "script_shape.h"
#include "script_tree_node.h"

#include <TDF_Tool.hxx>

#include <QtCore/QtDebug>
#include <QtQml/QJSEngine>

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

QVariant ScriptDocument::treeNode(unsigned int treeNodeId) const
{
    return QVariant::fromValue(ScriptTreeNode(m_doc, treeNodeId));
}

void ScriptDocument::traverseShape(QJSValue shape, unsigned shapeTypeFilter, QJSValue fn)
{
    if (!fn.isCallable())
        return;

    const auto shapeTypeEnum = static_cast<TopAbs_ShapeEnum>(shapeTypeFilter);
    const auto scriptShape = m_jsApp->jsEngine()->fromScriptValue<ScriptShape>(shape);
    BRepUtils::forEachSubShape(scriptShape.shape(), shapeTypeEnum, [&](const TopoDS_Shape& subShape) {
        auto jsSubShape = m_jsApp->jsEngine()->toScriptValue(ScriptShape(subShape));
        auto jsVal = fn.call({ jsSubShape });
        logScriptError(jsVal, "traverseShape()");
    });
}

bool ScriptDocument::importFile(QString strFilepath, QJSValue jsonOptions, QJSValue fnProgressCallback)
{
    const IO::System* ioSystem = m_jsApp->ioSystem();
    if (!ioSystem)
        return false;

    bool okImport = false;
    TaskManager taskMgr;
    auto taskId = taskMgr.newTask([&](TaskProgress* progress) {
        okImport =
            ioSystem->importInDocument()
                .targetDocument(this->baseDocument())
                .withFilepath(filepathFrom(strFilepath))
                .withEntityPostProcessRequiredIf(&IO::formatProvidesBRep)
                .withEntityPostProcessInfoProgress(20, "Mesh BRep shapes")
                .withTaskProgress(progress)
                .execute()
            ;
    });

    QString taskStep;
    int taskPct = 0;
    taskMgr.signalProgressStep.connect([&](TaskId, const std::string& step) {
        taskStep = to_QString(step);
        fnProgressCallback.call({ taskStep, taskPct });
    });
    taskMgr.signalProgressChanged.connect([&](TaskId, int pct) {
        taskPct = pct;
        fnProgressCallback.call({ taskStep, taskPct });
    });
    taskMgr.exec(taskId);
    return okImport;
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

} // namespace Mayo
