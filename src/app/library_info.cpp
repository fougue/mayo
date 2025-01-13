/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "library_info.h"

#include <vector>

namespace Mayo {

namespace {

std::vector<LibraryInfo>& getLibraryInfos()
{
    static std::vector<LibraryInfo> vec;
    return vec;
}

} // namespace

void LibraryInfoArray::add(
    std::string_view libName,
    std::string_view version,
    std::string_view versionDetails
    )
{
    if (!libName.empty() && !version.empty()) {
        const LibraryInfo libInfo{
            std::string(libName),
            std::string(version),
            std::string(versionDetails)
        };
        getLibraryInfos().push_back(std::move(libInfo));
    }
}

Span<const LibraryInfo> LibraryInfoArray::get()
{
    return getLibraryInfos();
}

} // namespace Mayo
