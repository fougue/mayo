/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <string>

namespace Mayo {

// Provides information about a 3rd-party library used by the application
struct LibraryInfo {
    // Precise name of the library
    std::string name;
    // Version string, can be codenamed and/or semantic
    std::string version;
    // Addition details about the version, such as compilation flags
    std::string versionDetails;
};

} // namespace Mayo
