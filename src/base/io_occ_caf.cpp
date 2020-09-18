/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_caf.h"
#include "document.h"
#include "occ_progress_indicator.h"
#include "scope_import.h"
#include "task_progress.h"

#include <Transfer_TransientProcess.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <gsl/gsl_util>

namespace Mayo {
namespace IO {

std::mutex& cafGlobalMutex()
{
    static std::mutex mutex;
    return mutex;
}

Handle_XSControl_WorkSession cafWorkSession(const STEPCAFControl_Reader& reader) {
    return reader.Reader().WS();
}

Handle_XSControl_WorkSession cafWorkSession(const IGESCAFControl_Reader& reader) {
    return reader.WS();
}

namespace {

template<typename CAF_READER>
bool cafGenericReadFile(CAF_READER& reader, const QString& filepath, TaskProgress* progress)
{
    std::lock_guard<std::mutex> lock(cafGlobalMutex()); Q_UNUSED(lock);
    //readFile_prepare(reader);
    const IFSelect_ReturnStatus error = reader.ReadFile(filepath.toLocal8Bit().constData());
    progress->setValue(100);
    return error == IFSelect_RetDone;
}

template<typename CAF_READER>
bool cafGenericReadTransfer(CAF_READER& reader, DocumentPtr doc, TaskProgress* progress)
{
    std::lock_guard<std::mutex> lock(cafGlobalMutex()); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    Handle_XSControl_WorkSession ws = cafWorkSession(reader);
    ws->MapReader()->SetProgress(indicator);
    auto _ = gsl::finally([&]{ ws->MapReader()->SetProgress(nullptr); });

    XCafScopeImport import(doc);
    Handle_TDocStd_Document stdDoc = doc;
    const bool okTransfer = reader.Transfer(stdDoc);
    import.setConfirmation(okTransfer && !TaskProgress::isAbortRequested(progress));
    return okTransfer;
}

template<typename CAF_WRITER>
bool cafGenericWriteTransfer(CAF_WRITER& writer, Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    std::lock_guard<std::mutex> lock(cafGlobalMutex()); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    cafFinderProcess(writer)->SetProgress(indicator);
    auto _ = gsl::finally([&]{ cafFinderProcess(writer)->SetProgress(nullptr); });
    for (const ApplicationItem& item : appItems) {
        bool okItemTransfer = false;
        if (item.isDocument())
            okItemTransfer = writer.Transfer(item.document());
        else if (item.isDocumentTreeNode())
            okItemTransfer = writer.Transfer(item.documentTreeNode().label());

        if (!okItemTransfer)
            return false;
    }

    return true;
}

} // namespace

bool cafReadFile(IGESCAFControl_Reader& reader, const QString& filepath, TaskProgress* progress) {
    return cafGenericReadFile(reader, filepath, progress);
}

bool cafReadFile(STEPCAFControl_Reader& reader, const QString& filepath, TaskProgress* progress) {
    return cafGenericReadFile(reader, filepath, progress);
}

bool cafTransfer(IGESCAFControl_Reader& reader, DocumentPtr doc, TaskProgress* progress) {
    return cafGenericReadTransfer(reader, doc, progress);
}

bool cafTransfer(STEPCAFControl_Reader& reader, DocumentPtr doc, TaskProgress* progress) {
    return cafGenericReadTransfer(reader, doc, progress);
}

Handle_Transfer_FinderProcess cafFinderProcess(const IGESCAFControl_Writer& writer) {
    return writer.TransferProcess();
}

Handle_Transfer_FinderProcess cafFinderProcess(const STEPCAFControl_Writer& writer) {
    return writer.Writer().WS()->TransferWriter()->FinderProcess();
}

bool cafTransfer(IGESCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgress* progress) {
    return cafGenericWriteTransfer(writer, appItems, progress);
}

bool cafTransfer(STEPCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgress* progress) {
    return cafGenericWriteTransfer(writer, appItems, progress);
}

} // namespace IO
} // namespace Mayo
