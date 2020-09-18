/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "document_ptr.h"
#include "span.h"

#include <Transfer_FinderProcess.hxx>
#include <XSControl_WorkSession.hxx>
#include <mutex>
class IGESCAFControl_Reader;
class STEPCAFControl_Reader;

class IGESCAFControl_Writer;
class STEPCAFControl_Writer;

class QString;

namespace Mayo {

class TaskProgress;

namespace IO {

std::mutex& cafGlobalMutex();

Handle_XSControl_WorkSession cafWorkSession(const IGESCAFControl_Reader& reader);
Handle_XSControl_WorkSession cafWorkSession(const STEPCAFControl_Reader& reader);

Handle_Transfer_FinderProcess cafFinderProcess(const IGESCAFControl_Writer& writer);
Handle_Transfer_FinderProcess cafFinderProcess(const STEPCAFControl_Writer& writer);

bool cafReadFile(IGESCAFControl_Reader& reader, const QString& filepath, TaskProgress* progress);
bool cafReadFile(STEPCAFControl_Reader& reader, const QString& filepath, TaskProgress* progress);

bool cafTransfer(IGESCAFControl_Reader& reader, DocumentPtr doc, TaskProgress* progress);
bool cafTransfer(STEPCAFControl_Reader& reader, DocumentPtr doc, TaskProgress* progress);

bool cafTransfer(IGESCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgress* progress);
bool cafTransfer(STEPCAFControl_Writer& writer, Span<const ApplicationItem> appItems, TaskProgress* progress);

} // namespace IO
} // namespace Mayo
