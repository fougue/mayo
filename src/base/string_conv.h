/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_handle.h"
#include <NCollection_UtfString.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <locale>
#include <string>
#include <string_view>

namespace Mayo {

// Base converter between string types
// Note: unicode(utf8/utf16) conversion is preferred when possible
template<typename InputStringType, typename OutputStringType> struct StringConv {};

// --
// -- General API
// --

// X -> Y
// Note: 'InputStringType' should be automatically deduced by the compiler
template<typename OutputStringType, typename InputStringType>
OutputStringType string_conv(const InputStringType& str) {
    return StringConv<InputStringType, OutputStringType>::to(str);
}


// X -> std::string
template<typename StringType>
std::string to_stdString(const StringType& str) {
    return string_conv<std::string>(str);
}

// X -> std::string_view
template<typename StringType>
std::string_view to_stdStringView(const StringType& str) {
    return string_conv<std::string_view>(str);
}

// X -> TCollection_AsciiString
template<typename StringType>
TCollection_AsciiString to_OccAsciiString(const StringType& str) {
    return string_conv<TCollection_AsciiString>(str);
}

// X -> Handle(TCollection_HAsciiString)
template<typename StringType>
OccHandle<TCollection_HAsciiString> to_OccHandleHAsciiString(const StringType& str) {
    return string_conv<OccHandle<TCollection_HAsciiString>>(str);
}

// X -> TCollection_ExtendedString
template<typename StringType>
TCollection_ExtendedString to_OccExtString(const StringType& str) {
    return string_conv<TCollection_ExtendedString>(str);
}

// double -> std::string

struct DoubleToStringOptions {
    std::locale locale;
    int decimalCount = 6;
    bool removeTrailingZeroes = true;
    bool roundToZero = true;
    bool toUtf8 = true;
    // double zeroPrecision = 0.000000000001;
};

class DoubleToStringOperation {
public:
    DoubleToStringOperation(double value);
    DoubleToStringOperation& locale(const std::locale& l);
    DoubleToStringOperation& decimalCount(int c);
    DoubleToStringOperation& removeTrailingZeroes(bool on);
    DoubleToStringOperation& roundToZero(bool on);
    DoubleToStringOperation& toUtf8(bool on);
    operator std::string();
    std::string get() const;

private:
    double m_value;
    DoubleToStringOptions m_opts;
};

std::string to_stdString(double value, const DoubleToStringOptions& opts);
DoubleToStringOperation to_stdString(double value);

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
        const TCollection_AsciiString asciiStr(str.data(), int(str.size()));
        return TCollection_ExtendedString(asciiStr, true/*multi-byte*/);
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

// std::string_view -> Handle(TCollection_HAsciiString)
template<> struct StringConv<std::string_view, OccHandle<TCollection_HAsciiString>> {
    static auto to(std::string_view str) {
        return makeOccHandle<TCollection_HAsciiString>(to_OccAsciiString(str));
    }
};

// std::string_view -> NCollection_Utf8String
template<> struct StringConv<std::string_view, NCollection_Utf8String> {
    static auto to(std::string_view str) {
        return NCollection_Utf8String(str.data(), static_cast<int>(str.size()));
    }
};

// --
// -- Handle(TCollection_HAsciiString) -> X
// --

// Handle(TCollection_HAsciiString) -> std::string
template<> struct StringConv<OccHandle<TCollection_HAsciiString>, std::string> {
    static auto to(const OccHandle<TCollection_HAsciiString>& str) {
        return string_conv<std::string>(str ? str->String() : TCollection_AsciiString());
    }
};

// Handle(TCollection_HAsciiString) -> std::string_view
template<> struct StringConv<OccHandle<TCollection_HAsciiString>, std::string_view> {
    static auto to(const OccHandle<TCollection_HAsciiString>& str) {
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
template<> struct StringConv<std::string, OccHandle<TCollection_HAsciiString>> {
    static auto to(const std::string& str) {
        return makeOccHandle<TCollection_HAsciiString>(str.c_str());
    }
};

// std::string -> TCollection_ExtendedString
template<> struct StringConv<std::string, TCollection_ExtendedString> {
    static auto to(const std::string& str) {
        return TCollection_ExtendedString(str.c_str(), true/*multi-byte*/);
    }
};

// std::string -> NCollection_Utf8String
template<> struct StringConv<std::string, NCollection_Utf8String> {
    static auto to(const std::string& str) {
        return NCollection_Utf8String(str.c_str(), static_cast<int>(str.size()));
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
