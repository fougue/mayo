/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Standard_Handle.hxx>
#include <utility> // For std::forward()

namespace Mayo {

// Template alias for OpenCascade handle
template<typename T> using OccHandle = opencascade::handle<T>;

// Constructs an object of 'T' wrapped in an OpenCascade handle
// Note: Standard_Transient must be a base class of 'T'
template<typename T, typename... Args>
OccHandle<T> makeOccHandle(Args&&... args)
{
    return new T(std::forward<Args>(args)...);
}

} // namespace Mayo
