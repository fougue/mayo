/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/global.h"
#ifdef MAYO_OS_WINDOWS
#  include <Windows.h>
#endif

#include "qstring_conv.h"

namespace Mayo {

std::string consoleToPrintable(const QString& str)
{
#ifdef MAYO_OS_WINDOWS
    const auto codepage = GetConsoleOutputCP();
    const wchar_t* source = reinterpret_cast<const wchar_t*>(str.utf16());
    const int dstSize = WideCharToMultiByte(codepage, 0, source, -1, nullptr, 0, nullptr, nullptr);
    std::string dst;
    dst.resize(dstSize + 1);
    WideCharToMultiByte(codepage, 0, source, -1, dst.data(), dstSize, nullptr, nullptr);
    dst.back() = '\0';
    return dst;
#else
    return str.toStdString(); // utf8
#endif
}

std::string consoleToPrintable(std::string_view str)
{
#ifdef MAYO_OS_WINDOWS
    return consoleToPrintable(to_QString(str));
#else
    return std::string(str); // utf8
#endif
}

} // namespace Mayo
