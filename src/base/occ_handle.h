/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Standard_Handle.hxx>

namespace Mayo {

// Template alias for OpenCascade handle
template<typename T> using OccHandle = opencascade::handle<T>;

} // namespace Mayo
