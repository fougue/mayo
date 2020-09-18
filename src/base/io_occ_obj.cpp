/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_obj.h"
#include "scope_import.h"
#include "occ_progress_indicator.h"
#include "task_progress.h"
#include <fougtools/occtools/qt_utils.h>

namespace Mayo {
namespace IO {

bool OccObjReader::readFile(const QString& filepath, TaskProgress* progress)
{
    m_filepath = filepath;
    progress->setValue(100);
    return true;
}

bool OccObjReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    m_reader.SetDocument(doc);
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    XCafScopeImport import(doc);
    const bool okPerform = m_reader.Perform(occ::QtUtils::toOccUtf8String(m_filepath), indicator);
    import.setConfirmation(okPerform && !TaskProgress::isAbortRequested(progress));
    return okPerform;
}

} // namespace IO
} // namespace Mayo
