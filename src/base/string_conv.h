/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <string>
#include <string_view>

namespace Mayo {

// Base converter between string types
// Note: unicode(utf8/utf16) conversion is preferred when possible
template<typename IN_STRING_TYPE, typename OUT_STRING_TYPE> struct StringConv {};

// --
// -- General API
// --

// X -> Y
// Note: 'IN_STRING_TYPE' should be automatically deduced by the compiler
template<typename OUT_STRING_TYPE, typename IN_STRING_TYPE>
OUT_STRING_TYPE string_conv(const IN_STRING_TYPE& str) {
    return StringConv<IN_STRING_TYPE, OUT_STRING_TYPE>::to(str);
}


// X -> std::string
template<typename STRING_TYPE>
std::string to_stdString(const STRING_TYPE& str) {
    return string_conv<std::string>(str);
}

// X -> std::string_view
template<typename STRING_TYPE>
std::string_view to_stdStringView(const STRING_TYPE& str) {
    return string_conv<std::string_view>(str);
}

// X -> TCollection_AsciiString
template<typename STRING_TYPE>
TCollection_AsciiString to_OccAsciiString(const STRING_TYPE& str) {
    return string_conv<TCollection_AsciiString>(str);
}

// X -> TCollection_ExtendedString
template<typename STRING_TYPE>
TCollection_ExtendedString to_OccExtString(const STRING_TYPE& str) {
    return string_conv<TCollection_ExtendedString>(str);
}


// --
// -- Converters(misc)
// --

// const char* -> TCollection_ExtendedString
template<> struct StringConv<const char*, TCollection_ExtendedString> {
    static auto to(const char* str) {
        return TCollection_ExtendedString(str, true/*multi-byte*/);
    }
};

// const char[N] -> TCollection_ExtendedString
template<size_t N> struct StringConv<char[N], TCollection_ExtendedString> {
    static auto to(const char(&str)[N]) {
        return TCollection_ExtendedString(str, true/*multi-byte*/);
    }
};


#if 0
// std::string_view -> TCollection_ExtendedString
template<> struct StringConv<std::string_view, TCollection_ExtendedString> {
    static auto to(std::string_view str) {
        return TCollection_ExtendedString(str.data(), true/*multi-byte*/);
    }
};
#endif

// --
// -- std::string_view -> X
// --

// std::string_view -> TCollection_AsciiString
template<> struct StringConv<std::string_view, TCollection_AsciiString> {
    static auto to(std::string_view str) {
        return TCollection_AsciiString(str.data(), int(str.size()));
    }
};

// --
// -- Handle(TCollection_HAsciiString) -> X
// --

// Handle(TCollection_HAsciiString) -> std::string
template<> struct StringConv<Handle(TCollection_HAsciiString), std::string> {
    static auto to(const Handle(TCollection_HAsciiString)& str) {
        return string_conv<std::string>(str ? str->String() : TCollection_AsciiString());
    }
};

// Handle(TCollection_HAsciiString) -> std::string_view
template<> struct StringConv<Handle(TCollection_HAsciiString), std::string_view> {
    static auto to(const Handle(TCollection_HAsciiString)& str) {
        return string_conv<std::string_view>(str ? str->String() : TCollection_AsciiString());
    }
};

// --
// -- std::string -> X
// --

// std::string -> TCollection_AsciiString
template<> struct StringConv<std::string, TCollection_AsciiString> {
    static auto to(const std::string& str) {
        return TCollection_AsciiString(str.c_str(), int(str.length()));
    }
};

// std::string -> Handle(TCollection_HAsciiString)
template<> struct StringConv<std::string, Handle(TCollection_HAsciiString)> {
    static auto to(const std::string& str) {
        Handle(TCollection_HAsciiString) hnd = new TCollection_HAsciiString(str.c_str());
        return hnd;
    }
};

// std::string -> TCollection_ExtendedString
template<> struct StringConv<std::string, TCollection_ExtendedString> {
    static auto to(const std::string& str) {
        return TCollection_ExtendedString(str.c_str(), true/*multi-byte*/);
    }
};

// --
// -- TCollection_AsciiString -> X
// --

// TCollection_AsciiString -> std::string
template<> struct StringConv<TCollection_AsciiString, std::string> {
    static auto to(const TCollection_AsciiString& str) {
        return std::string(str.ToCString(), str.Length());
    }
};

// TCollection_AsciiString -> std::string_view
template<> struct StringConv<TCollection_AsciiString, std::string_view> {
    static auto to(const TCollection_AsciiString& str) {
        return std::string_view(str.ToCString(), str.Length());
    }
};

// --
// -- TCollection_ExtendedString -> X
// --

// TCollection_ExtendedString -> std::string
template<> struct StringConv<TCollection_ExtendedString, std::string> {
    static auto to(const TCollection_ExtendedString& str) {
        std::string u8;
        u8.resize(str.LengthOfCString());
        char* u8Data = u8.data();
        str.ToUTF8CString(u8Data);
        return u8;
    }
};

// TCollection_ExtendedString -> std::u16string
template<> struct StringConv<TCollection_ExtendedString, std::u16string> {
    static auto to(const TCollection_ExtendedString& str) {
        return std::u16string(str.ToExtString(), str.Length());
    }
};

// TCollection_ExtendedString -> std::u16string_view
template<> struct StringConv<TCollection_ExtendedString, std::u16string_view> {
    static auto to(const TCollection_ExtendedString& str) {
        return std::u16string_view(str.ToExtString(), str.Length());
    }
};

} // namespace Mayo
