/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Handle.hxx>
#include <Standard_Version.hxx>
#include <utility> // For std::forward()

namespace Mayo {

// Template alias for OpenCascade handle
template<typename T> using OccHandle = opencascade::handle<T>;

// Constructs an object of type 'T' wrapped in an OpenCascade handle
// Note: Standard_Transient must be a base class of 'T'
template<typename T, typename... Args>
OccHandle<T> makeOccHandle(Args&&... args)
{
    return new T(std::forward<Args>(args)...);
}

} // namespace Mayo

#if OCC_VERSION_HEX < 0x070800
namespace std {

// Specialization of C++11 std::hash<> functor for opencascade::handle<> objects
template<typename T> struct hash<opencascade::handle<T>> {
    inline std::size_t operator()(const opencascade::handle<T>& hnd) const {
        return hash<T*>{}(hnd.get());
    }
};

} // namespace std
#endif
