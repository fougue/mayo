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

class QString;

namespace Mayo {

class TaskProgressPortion;

namespace IO {
namespace Private {

std::mutex& cafGlobalMutex();

#define MayoIO_CafGlobalScopedLock(name) \
    std::lock_guard<std::mutex> name(Mayo::IO::Private::cafGlobalMutex()); \
    Q_UNUSED(name);

Handle_XSControl_WorkSession cafWorkSession(const IGESCAFControl_Reader& reader);
Handle_XSControl_WorkSession cafWorkSession(const STEPCAFControl_Reader& reader);

Handle_Transfer_FinderProcess cafFinderProcess(const IGESCAFControl_Writer& writer);
Handle_Transfer_FinderProcess cafFinderProcess(const STEPCAFControl_Writer& writer);

bool cafReadFile(IGESCAFControl_Reader& reader, const FilePath& filepath, TaskProgressPortion* progress);
bool cafReadFile(STEPCAFControl_Reader& reader, const FilePath& filepath, TaskProgressPortion* progress);

bool cafTransfer(IGESCAFControl_Reader& reader, DocumentPtr doc, TaskProgressPortion* progress);
bool cafTransfer(STEPCAFControl_Reader& reader, DocumentPtr doc, TaskProgressPortion* progress);

bool cafTransfer(IGESCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgressPortion* progress);
bool cafTransfer(STEPCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgressPortion* progress);

} // namespace Private
} // namespace IO
} // namespace Mayo
