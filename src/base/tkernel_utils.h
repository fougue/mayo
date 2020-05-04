/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Standard_Handle.hxx>

namespace Mayo {

class TKernelUtils {
public:
    template<typename STD_TRANSIENT>
    opencascade::handle<T> makeHandle(const STD_TRANSIENT* ptr) { return ptr; }
};

} // namespace Mayo
