/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/span.h"

#include <functional>

namespace Mayo {

class Application;

// Contains arguments for the cli_asyncExportDocuments() function
struct CliExportArgs {
    bool progressReport = true;
    Span<const FilePath> filesToOpen;
    Span<const FilePath> filesToExport;
};

// Asynchronously exports input file(s) listed in 'args'
// Calls 'fnContinuation' at the end of execution
void cli_asyncExportDocuments(
        Application* app,
        const CliExportArgs& args,
        std::function<void(int)> fnContinuation
);

} // namespace Mayo
