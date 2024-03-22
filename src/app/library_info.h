/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/span.h"

#include <string>
#include <string_view>

namespace Mayo {

struct LibraryInfo {
    std::string name;
    std::string version;
    std::string versionDetails;
};

struct LibraryInfoArray {
    static void add(
        std::string_view libName,
        std::string_view version,
        std::string_view versionDetails = ""
    );

    static Span<const LibraryInfo> get();
};

} // namespace Mayo
