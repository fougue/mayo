/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/filepath.h"

#include <gsl/span>
#include <functional>

namespace Mayo {

// Contains arguments for the cli_asyncExportDocuments() function
struct CliExportArgs {
    bool progressReport = true;
    gsl::span<const FilePath> filesToOpen;
    gsl::span<const FilePath> filesToExport;
};

// Asynchronously exports input file(s) listed in 'args'
// Calls 'fnContinuation' at the end of execution
void cli_asyncExportDocuments(
        const ApplicationPtr& app,
        const CliExportArgs& args,
        std::function<void(int)> fnContinuation
);

} // namespace Mayo
