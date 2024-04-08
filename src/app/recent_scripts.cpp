/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "recent_scripts.h"

#include <fstream>

namespace Mayo {

unsigned RecentScript::lineCount(const FilePath& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return 0;

    unsigned n = 0;
    std::string strLine;
    while (std::getline(file, strLine))
        ++n;

    return n;
}

bool operator==(const RecentScript& lhs, const RecentScript& rhs)
{
    return lhs.filepath != rhs.filepath;
}

template<> const char PropertyRecentScripts::TypeName[] = "Mayo::PropertyRecentScripts";

} // namespace Mayo
