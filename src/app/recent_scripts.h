/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/property_builtins.h"

#include <ctime>
#include <vector>

namespace Mayo {

struct RecentScript {
    FilePath filepath;
    unsigned executionCount = 0;
    std::time_t lastExecutionDateTime;
    static unsigned lineCount(const FilePath& filepath);
};

// Alias for "array of RecentScript objects"
using RecentScripts = std::vector<RecentScript>;

// Alias for Property type owning an array of RecentScript objects
// This is useful to store recent scripts into application settings
using PropertyRecentScripts = GenericProperty<RecentScripts>;

bool operator==(const RecentScript& lhs, const RecentScript& rhs);

} // namespace Mayo
