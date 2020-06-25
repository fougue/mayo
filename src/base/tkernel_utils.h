/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Standard_Handle.hxx>
#include <Standard_Version.hxx>

#ifndef OCC_VERSION_CHECK
#  define OCC_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#endif

namespace Mayo {

class TKernelUtils {
public:
    template<typename STD_TRANSIENT>
    opencascade::handle<STD_TRANSIENT> makeHandle(const STD_TRANSIENT* ptr) { return ptr; }
};

} // namespace Mayo
