/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_mayo.h"
#include "script_application.h"
#include "script_document.h"

#include "../base/brep_utils.h"
#include "../base/document.h"
#include "../qtcommon/qstring_conv.h"

#include <common/mayo_version.h>

#include <QtCore/QElapsedTimer>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtQml/QJSEngine>

namespace Mayo {

static_assert(std::is_same_v<TaskId, uint32_t>);
static_assert(std::is_same_v<TreeNodeId, uint32_t>);

ScriptMayo::ScriptMayo(const ScriptEnvironment& scriptEnv, QJSEngine* jsEngine)
    : QObject(jsEngine),
    m_scriptEnv(scriptEnv),
    m_jsEngine(jsEngine)
{
    m_taskMgr.signalStarted.connectSlot([&](TaskId taskId) {
        Task* task = this->findTask(taskId);
        if (task && task->callbacks.onStarted.isCallable())
            task->callbacks.onStarted.call({taskId});
    });
    m_taskMgr.signalProgressStep.connectSlot([=](TaskId taskId, const std::string& step) {
        Task* task = this->findTask(taskId);
        if (task && task->callbacks.onProgress.isCallable()) {
            task->progressStepTitle = to_QString(step);
            task->callbacks.onProgress.call({ task->progressStepTitle, task->progressPct, taskId });
        }
    });
    m_taskMgr.signalProgressChanged.connectSlot([&](TaskId taskId, int pct) {
        Task* task = this->findTask(taskId);
        if (task && task->callbacks.onProgress.isCallable()) {
            task->progressPct = pct;
            task->callbacks.onProgress.call({ task->progressStepTitle, task->progressPct, taskId });
        }
    });
    m_taskMgr.signalEnded.connectSlot([&](TaskId taskId) {
        Task* task = this->findTask(taskId);
        if (task && task->callbacks.onEnded.isCallable())
            task->callbacks.onEnded.call({taskId});
        m_mapTask.erase(taskId);
    });
}

QString ScriptMayo::versionString() const
{
    return to_QString(Mayo::strVersion);
}

/*!
  \brief Waits up to `msecs` milliseconds for all running asynchronous tasks to complete

  Returns `true` if all tasks were finished; otherwise it returns `false`.<br>
  If `msecs` is -1(the default), the timeout is ignored(waits for the last task to exit).
*/
bool ScriptMayo::waitForDone(int msecs)
{
    QJSValueList taskIdList;
    for (const auto& [taskId, task] : m_mapTask)
        taskIdList.push_back(taskId);

    return this->waitForDone(taskIdList, msecs);
}

/*!
  \brief Waits up to `msecs` milliseconds for a specific running asynchronous tasks to complete

  Returns `true` if the task identified by `taskId` was finished; otherwise it returns `false`.<br>
  If `msecs` is -1(the default), the timeout is ignored(waits for the task to exit).
*/
bool ScriptMayo::waitForDone(TaskId taskId, int msecs)
{
    QEventLoop eventLoop;
    int waitAmount = 0;
    const int waitIncrement = 250;
    auto fnTimeout = [&]{ return msecs > 0 ? waitAmount >= msecs : false; };
    while (!fnTimeout() && !m_taskMgr.waitForDone(taskId, 10/*ms*/)) {
        QTimer::singleShot(waitIncrement, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
        waitAmount += waitIncrement;
    }

    return !fnTimeout();
}

/*!
  \brief Waits up to `msecs` milliseconds for some running asynchronous tasks to complete

  Returns `true` if the tasks identified by `taskIdArray` were finished; otherwise it returns
  `false`.<br>
  If `msecs` is -1(the default), the timeout is ignored(waits for the tasks to exit).
*/
bool ScriptMayo::waitForDone(QJSValueList_ArrayOfTaskId taskIdArray, int msecs)
{
    QElapsedTimer chrono;
    chrono.start();
    for (const QJSValue& taskId : taskIdArray) {
        if (msecs > 0 && chrono.elapsed() >= msecs)
            return false;

        if (taskId.isNumber()) {
            const int maxDuration = msecs > 0 ? msecs - chrono.elapsed() : -1;
            if (!this->waitForDone(taskId.toUInt(), maxDuration))
                return false;
        }
    }

    return true;
}

/*!
  \brief Visits each node in the Document model tree and executes callback `fn` on the visited node

  `MayoTraverseModelTreeCallback` is a unary callback function which is passed the TreeNodeId
  value of the visited tree node. Any value returned by the callback is ignored
  <br>Example:
  \code{.js}
    Mayo.traverseModelTree(doc, nodeId => {
        var node = doc.treeNode(nodeId);
        console.debug(`treeNodeId: ${nodeId}  name:"${node.name}"`);
    });
  \endcode
*/
void ScriptMayo::traverseModelTree(
        QJSValue_ScriptDocument jsDoc, QJSValue_MayoTraverseModelTreeCallback fn
    )
{
    if (!fn.isCallable())
        return;

    auto scriptDoc = qobject_cast<ScriptDocument*>(jsDoc.toQObject());
    if (!scriptDoc)
        return;

    const Tree<TDF_Label>& modelTree = scriptDoc->document()->modelTree();
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

/*!
  \brief Visits each sub-shape and executes callback `fn` on the visited sub-shape

  The sub-shapes visited are restricted by a shape type filter specified with `filter`.
  This means that if for example ShapeType.Edge is passed then only the sub-shapes of type "edge"
  will be visited\n
  `MayoTraverseShapeCallback` is a unary callback function which is passed the Shape object of the
  visited sub-shape. Any value returned by the callback is ignored
  <br>Example:
  \code{.js}
    var circleCount = 0;
    Mayo.traverseShape(shape, ShapeType.Edge, edge => {
        if (edge.geometry.type == GeomCurveType.Circle)
            ++circleCount;
    });
    console.debug("Circle count = " + circleCount);
  \endcode
*/
void ScriptMayo::traverseShape(
        QJSValue_ScriptShape jsShape, ScriptShapeType filter, QJSValue_MayoTraverseShapeCallback fn
    )
{
    if (!m_jsEngine)
        return; // TODO Handle error(throw exception?)

    if (!fn.isCallable())
        return;

    const auto shapeTypeEnum = static_cast<TopAbs_ShapeEnum>(filter);
    const auto scriptShape = m_jsEngine->fromScriptValue<ScriptShape>(jsShape);
    BRepUtils::forEachSubShape(scriptShape.shape(), shapeTypeEnum, [&](const TopoDS_Shape& subShape) {
        auto jsSubShape = m_jsEngine->toScriptValue(ScriptShape(subShape));
        auto jsVal = fn.call({ jsSubShape });
        logScriptError(jsVal, "Mayo.traverseShape()");
    });
}

void ScriptMayo::registerTask(
        TaskId taskId, const TaskCallbacks& callbacks, std::unique_ptr<MessengerBySignal> msg
    )
{
    m_mapTask.insert({ taskId, Task{} });
    Task* task = this->findTask(taskId);
    task->callbacks = callbacks;
    task->messenger = std::move(msg);
    task->messenger->signalMessage.connectSlot([=](MessageType msgType, std::string msg) {
        const QJSValueList jsCallbackArgs{ to_QString(msg), taskId };
        switch (msgType) {
        case MessageType::Info: {
            if (task->callbacks.onInfo.isCallable())
                task->callbacks.onInfo.call(jsCallbackArgs);
            break;
        }
        case MessageType::Warning: {
            if (task->callbacks.onWarning.isCallable())
                task->callbacks.onWarning.call(jsCallbackArgs);
            break;
        }
        case MessageType::Error: {
            if (task->callbacks.onError.isCallable())
                task->callbacks.onError.call(jsCallbackArgs);
            break;
        }
        } // endswitch
    });
}

ScriptMayo::Task* ScriptMayo::findTask(TaskId taskId)
{
    auto it = m_mapTask.find(taskId);
    return it != m_mapTask.cend() ? &it->second : nullptr;
}

void ScriptMayo::setMainApplication(const ApplicationPtr& app)
{
    delete m_mainApplication;
    m_mainApplication = new ScriptApplication(app, this);
}

} // namespace Mayo
