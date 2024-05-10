/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "cli_export.h"

#include "console.h"
#include "../app/app_module.h"
#include "../base/application.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/task_manager.h"
#include "../qtcommon/qstring_conv.h"

#include <Message.hxx>

#include <QtCore/QtDebug>

#include <fmt/format.h>
#include <atomic>
#include <functional>
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace Mayo {

class CliExport {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::CliExport)
};

namespace {

// Provides thread-safe status of a task
struct TaskStatus {
    std::atomic<bool> finished = {};
    std::atomic<bool> success = {};
};

// Provides helper data that exists during execution of cli_asyncExportDocuments() function
struct Helper : public QObject {
    // Task manager object to be used
    TaskManager taskMgr;
    // Counter decremented for each finished export task, when 0 is reached then quit
    std::atomic<int> exportTaskCount = {};
    // Mapping between a task id and the task status
    std::unordered_map<TaskId, std::unique_ptr<TaskStatus>> mapTaskStatus;
    // Mapping between a task id and corresponding width of the progress line in console
    std::unordered_map<TaskId, int> mapTaskLineWidth;
    // Count of progress lines in console after last call to printTaskProgress()
    int lastPrintProgressLineCount = 0;
};

// Collects emitted error messages into a single string object
class ErrorMessageCollect : public Messenger {
public:
    void emitMessage(MessageType msgType, std::string_view text) override
    {
        if (msgType == MessageType::Error) {
            m_message += text;
            m_message += " ";
        }
    }

    const std::string& message() const { return m_message; }

private:
    std::string m_message;
};

void printTaskProgress(Helper* helper, TaskId taskId)
{
    std::string strMessage = helper->taskMgr.title(taskId);
    std::replace(strMessage.begin(), strMessage.end(), '\n', ' ');
    strMessage = consoleToPrintable(strMessage);
    auto lineWidth = int(strMessage.size());
    const bool taskFinished = helper->mapTaskStatus.at(taskId)->finished;
    const bool taskSuccess = helper->mapTaskStatus.at(taskId)->success;
    if (taskFinished && !taskSuccess) {
        consoleSetTextColor(ConsoleColor::Red);
        std::cout << strMessage;
        consoleSetTextColor(ConsoleColor::Default);
    }
    else {
        const int progress = helper->taskMgr.progress(taskId);
        if (progress >= 100)
            consoleSetTextColor(ConsoleColor::Green);

        std::cout << std::setfill(' ') << std::right << std::setw(3) << progress << "% ";
        std::cout << strMessage;
        lineWidth += 5;
        if (progress >= 100)
            consoleSetTextColor(ConsoleColor::Default);
    }

    const int printWidth = consoleWidth();
    auto itLineFound = helper->mapTaskLineWidth.find(taskId);
    const int lineWidthOld = itLineFound != helper->mapTaskLineWidth.cend() ? itLineFound->second : printWidth - 1;
    for (int i = 0; i < (lineWidthOld - lineWidth); ++i)
        std::cout << ' ';

    helper->mapTaskLineWidth.insert_or_assign(taskId, lineWidth);
    helper->lastPrintProgressLineCount += printWidth > 0 ? (lineWidth / printWidth) + 1 : 1;
    std::cout << "\n";
}

bool importInDocument(DocumentPtr doc, const CliExportArgs& args, Helper* helper, TaskProgress* progress)
{
    auto appModule = AppModule::get();

    // If export operation targets some mesh format then force meshing of imported BRep shapes
    bool brepMeshRequired = false;
    for (const FilePath& filepath : args.filesToExport) {
        const IO::Format format = appModule->ioSystem()->probeFormat(filepath);
        brepMeshRequired = IO::formatProvidesMesh(format);
        if (brepMeshRequired)
            break; // Interrupt
    }

    ErrorMessageCollect errorCollect;
    const bool okImport = appModule->ioSystem()->importInDocument()
        .targetDocument(doc)
        .withFilepaths(args.filesToOpen)
        .withParametersProvider(appModule)
        .withEntityPostProcess([=](TDF_Label labelEntity, TaskProgress* progress) {
            appModule->computeBRepMesh(labelEntity, progress);
        })
        .withEntityPostProcessRequiredIf([=](IO::Format){ return brepMeshRequired; })
        .withEntityPostProcessInfoProgress(20, CliExport::textIdTr("Mesh BRep shapes"))
        .withMessenger(&errorCollect)
        .withTaskProgress(progress)
        .execute();
    helper->taskMgr.setTitle(progress->taskId(), okImport ? CliExport::textIdTr("Imported") : errorCollect.message());
    helper->mapTaskStatus.at(progress->taskId())->success = okImport;
    helper->mapTaskStatus.at(progress->taskId())->finished = true;
    return okImport;
}

void exportDocument(const DocumentPtr& doc, const FilePath& filepath, Helper* helper, TaskProgress* progress)
{
    auto appModule = AppModule::get();
    ErrorMessageCollect errorCollect;
    const IO::Format format = appModule->ioSystem()->probeFormat(filepath);
    const ApplicationItem appItems[] = { doc };
    const bool okExport = appModule->ioSystem()->exportApplicationItems()
                .targetFile(filepath)
                .targetFormat(format)
                .withItems(appItems)
                .withParameters(appModule->findWriterParameters(format))
                .withMessenger(&errorCollect)
                .withTaskProgress(progress)
                .execute();
    const std::string strFilename = filepath.filename().u8string();
    const std::string msg =
            okExport ?
                fmt::format(CliExport::textIdTr("Exported {}"), strFilename) :
                errorCollect.message();
    helper->taskMgr.setTitle(progress->taskId(), msg);
    helper->mapTaskStatus.at(progress->taskId())->success = okExport;
    helper->mapTaskStatus.at(progress->taskId())->finished = true;
    --(helper->exportTaskCount);
}

} // namespace

void cli_asyncExportDocuments(
        const ApplicationPtr& app,
        const CliExportArgs& args,
        std::function<void(int)> fnContinuation
    )
{
    auto helper = new Helper; // Allocated on heap because current function is asynchronous
    auto taskMgr = &helper->taskMgr;

    // Helper function to exit current function
    auto fnExit = [=](int retCode) {
        helper->deleteLater();
        fnContinuation(retCode);
    };

    // Helper function to print in console the progress info of current function
    auto fnPrintProgress = [=]{
        consoleCursorMoveUp(helper->lastPrintProgressLineCount);
        helper->lastPrintProgressLineCount = 0;
        std::cout << "\r";
        taskMgr->foreachTask([=](TaskId taskId) { printTaskProgress(helper, taskId); });
        std::cout.flush();
    };

    // Show progress/traces corresponding to task events
    taskMgr->signalStarted.connectSlot([=](TaskId taskId) {
        if (args.progressReport)
            fnPrintProgress();
        else
            qInfo() << to_QString(taskMgr->title(taskId));
    });
    taskMgr->signalEnded.connectSlot([=](TaskId taskId) {
        if (args.progressReport) {
            fnPrintProgress();
        }
        else {
            if (helper->mapTaskStatus.at(taskId)->success)
                qInfo() << to_QString(taskMgr->title(taskId));
            else
                qCritical() << to_QString(taskMgr->title(taskId));
        }
    });
    taskMgr->signalProgressChanged.connectSlot([=]{
        if (args.progressReport)
            fnPrintProgress();
    });

    helper->exportTaskCount = int(args.filesToExport.size());
    taskMgr->signalEnded.connectSlot([=]{
        if (helper->exportTaskCount == 0) {
            bool okExport = true;
            for (const auto& mapPair : helper->mapTaskStatus) {
                const TaskStatus* status = mapPair.second.get();
                okExport = okExport && status->success;
            }

            fnExit(okExport ? EXIT_SUCCESS : EXIT_FAILURE);
        }
    });

    // Suppress output from OpenCascade
    Message::DefaultMessenger()->RemovePrinters(Message_Printer::get_type_descriptor());

    // Execute import operation(synchronous)
    DocumentPtr doc = app->newDocument();
    bool okImport = true;
    const TaskId importTaskId = taskMgr->newTask([&](TaskProgress* progress) {
            okImport = importInDocument(doc, args, helper, progress);
    });
    helper->mapTaskStatus.insert({ importTaskId, std::make_unique<TaskStatus>() });
    taskMgr->setTitle(importTaskId, CliExport::textIdTr("Importing..."));
    taskMgr->exec(importTaskId, TaskAutoDestroy::Off);
    if (!okImport)
        return fnExit(EXIT_FAILURE); // Error

    // Run export operations(asynchronous)
    for (const FilePath& filepath : args.filesToExport) {
        const TaskId taskId = taskMgr->newTask([=](TaskProgress* progress) {
            exportDocument(doc, filepath, helper, progress);
        });
        const std::string strFilename = filepath.filename().u8string();
        helper->mapTaskStatus.insert({ taskId, std::make_unique<TaskStatus>() });
        taskMgr->setTitle(taskId, fmt::format(CliExport::textIdTr("Exporting {}..."), strFilename));
    }

    taskMgr->foreachTask([=](TaskId taskId) {
        if (taskId != importTaskId)
            taskMgr->run(taskId, TaskAutoDestroy::Off);
    });
}

} // namespace Mayo
