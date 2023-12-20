/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Quantity_Color.hxx>
#include <Standard_Handle.hxx>
#include <Standard_Version.hxx>
#include <string>
#include <string_view>

#ifndef OCC_VERSION_CHECK
#  define OCC_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include <Message_ProgressRange.hxx>
#else
#  include <Standard_Handle.hxx>
class Message_ProgressIndicator;
#endif

namespace Mayo {

// Provides helper functions for OpenCascade TKernel library
class TKernelUtils {
public:
    template<typename TransientType>
    static opencascade::handle<TransientType> makeHandle(const TransientType* ptr) { return ptr; }

    using ReturnType_StartProgressIndicator =
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
                Message_ProgressRange;
#else
                const opencascade::handle<Message_ProgressIndicator>&;
#endif
    static ReturnType_StartProgressIndicator start(const opencascade::handle<Message_ProgressIndicator>& progress);

    // Encodes 'color' into hexadecimal representation with #RRGGBB format
    static std::string colorToHex(const Quantity_Color& color);

    // Decodes a string containing a color with #RRGGBB format to a Quantity_Color object
    // RR, GG, BB are in hexadecimal notation
    static bool colorFromHex(std::string_view strHex, Quantity_Color* color);

    // Returns the type to be used(by default) for RGB colors, depending on OpenCascasde version
    static Quantity_TypeOfColor preferredRgbColorType();

    // Returns a linear-space RGB color from input 'color' expressed with preferredRgbColorType()
    static Quantity_Color toLinearRgbColor(const Quantity_Color& color);
};

} // namespace Mayo

namespace std {

// Specialization of C++11 std::hash<> functor for opencascade::handle<> objects
template<typename T> struct hash<opencascade::handle<T>> {
    inline std::size_t operator()(const opencascade::handle<T>& hnd) const {
        return hash<T*>{}(hnd.get());
    }
};

} // namespace std
