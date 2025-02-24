/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// --
// -- Provides helper tools for IGESCAF/STEPCAF reader/writer
// --

#include "../base/application_item.h"
#include "../base/document_ptr.h"
#include "../base/filepath.h"
#include "../base/span.h"

#include <Transfer_FinderProcess.hxx>
#include <XSControl_WorkSession.hxx>
#include <mutex>
class IGESCAFControl_Reader;
class STEPCAFControl_Reader;

class IGESCAFControl_Writer;
class STEPCAFControl_Writer;

namespace Mayo { class TaskProgress; }

namespace Mayo::IO::Private {

std::mutex& cafGlobalMutex();

#define MayoIO_CafGlobalScopedLock(name) \
    [[maybe_unused]] std::lock_guard<std::mutex> name(Mayo::IO::Private::cafGlobalMutex());

OccHandle<XSControl_WorkSession> cafWorkSession(const IGESCAFControl_Reader& reader);
OccHandle<XSControl_WorkSession> cafWorkSession(const STEPCAFControl_Reader& reader);

OccHandle<Transfer_FinderProcess> cafFinderProcess(const IGESCAFControl_Writer& writer);
OccHandle<Transfer_FinderProcess> cafFinderProcess(const STEPCAFControl_Writer& writer);

bool cafReadFile(IGESCAFControl_Reader& reader, const FilePath& filepath, TaskProgress* progress);
bool cafReadFile(STEPCAFControl_Reader& reader, const FilePath& filepath, TaskProgress* progress);

TDF_LabelSequence cafTransfer(IGESCAFControl_Reader& reader, DocumentPtr doc, TaskProgress* progress);
TDF_LabelSequence cafTransfer(STEPCAFControl_Reader& reader, DocumentPtr doc, TaskProgress* progress);

bool cafTransfer(IGESCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgress* progress);
bool cafTransfer(STEPCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgress* progress);

} // namespace Mayo::IO::Private
