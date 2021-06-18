/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QString>
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

// X -> QString
template<typename STRING_TYPE>
QString to_QString(const STRING_TYPE& str) {
    return string_conv<QString>(str);
}

// X -> std::string
template<typename STRING_TYPE>
std::string to_stdString(const STRING_TYPE& str) {
    return string_conv<std::string>(str);
}

// X -> TCollection_ExtendedString
template<typename STRING_TYPE>
TCollection_ExtendedString to_OccExtString(const STRING_TYPE& str) {
    return string_conv<TCollection_ExtendedString>(str);
}


// --
// -- Converters
// --

// const char* -> TCollection_ExtendedString
template<> struct StringConv<const char*, TCollection_ExtendedString> {
    static auto to(const char* str) {
        return TCollection_ExtendedString(str, true/*multi-byte*/);
    }
};

// const char* -> QString
template<> struct StringConv<const char*, QString> {
    static auto to(const char* str) { return QString::fromUtf8(str); }
};

// const char[N] -> TCollection_ExtendedString
template<size_t N> struct StringConv<char[N], TCollection_ExtendedString> {
    static auto to(const char(&str)[N]) {
        return TCollection_ExtendedString(str, true/*multi-byte*/);
    }
};

// const char[N] -> QString
template<size_t N> struct StringConv<char[N], QString> {
    static auto to(const char(&str)[N]) { return QString::fromUtf8(str, N); }
};

// std::string -> TCollection_ExtendedString
template<> struct StringConv<std::string, TCollection_ExtendedString> {
    static auto to(const std::string& str) {
        return TCollection_ExtendedString(str.c_str(), true/*multi-byte*/);
    }
};

// std::string -> QString
template<> struct StringConv<std::string, QString> {
    static auto to(const std::string& str) { return QString::fromStdString(str); }
};

// std::string_view -> QString
template<> struct StringConv<std::string_view, QString> {
    static auto to(std::string_view str) { return QString::fromUtf8(str.data(), int(str.size())); }
};

// QString -> TCollection_AsciiString
template<> struct StringConv<QString, TCollection_AsciiString> {
    static auto to(const QString& str) {
        return TCollection_AsciiString(qUtf8Printable(str));
    }
};

// QString -> Handle_TCollection_HAsciiString
template<> struct StringConv<QString, Handle_TCollection_HAsciiString> {
    static auto to(const QString& str) {
        Handle_TCollection_HAsciiString hnd = new TCollection_HAsciiString(qUtf8Printable(str));
        return hnd;
    }
};

// QString -> opencascade::handle<TCollection_HAsciiString>
template<> struct StringConv<QString, opencascade::handle<TCollection_HAsciiString>> {
    static auto to(const QString& str) {
        opencascade::handle<TCollection_HAsciiString> hnd = new TCollection_HAsciiString(qUtf8Printable(str));
        return hnd;
    }
};

// QString -> Handle_TCollection_HAsciiString
template<> struct StringConv<Handle_TCollection_HAsciiString, QString> {
    static auto to(const Handle_TCollection_HAsciiString& str) {
        return string_conv<QString>(str ? str->String() : TCollection_AsciiString());
    }
};

// Handle_TCollection_HAsciiString -> QString
template<> struct StringConv<opencascade::handle<TCollection_HAsciiString>, QString> {
    static auto to(const opencascade::handle<TCollection_HAsciiString>& str) {
        return string_conv<QString>(str ? str->String() : TCollection_AsciiString());
    }
};

// QByteArray -> QString
template<> struct StringConv<QByteArray, QString> {
    static auto to(const QByteArray& str) { return QString::fromUtf8(str); }
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
// -- TCollection_AsciiString
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

// TCollection_AsciiString -> QLatin1String
template<> struct StringConv<TCollection_AsciiString, QLatin1String> {
    static auto to(const TCollection_AsciiString& str) {
        return QLatin1String(str.ToCString(), str.Length());
    }
};

// TCollection_AsciiString -> QByteArray
template<> struct StringConv<TCollection_AsciiString, QByteArray> {
    static auto to(const TCollection_AsciiString& str) {
        return QByteArray(str.ToCString(), str.Length());
    }
};

// TCollection_AsciiString -> QString
template<> struct StringConv<TCollection_AsciiString, QString> {
    static auto to(const TCollection_AsciiString& str) {
        return QString::fromUtf8(str.ToCString(), str.Length());
    }
};

// --
// -- TCollection_ExtendedString
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

// TCollection_ExtendedString -> QStringView
template<> struct StringConv<TCollection_ExtendedString, QStringView> {
    static auto to(const TCollection_ExtendedString& str) {
        return QStringView(str.ToExtString(), str.Length());
    }
};

// TCollection_ExtendedString -> QString
template<> struct StringConv<TCollection_ExtendedString, QString> {
    static auto to(const TCollection_ExtendedString& str) {
        return QString::fromUtf16(str.ToExtString(), str.Length());
    }
};

// --
// -- QString
// --

// QString -> std::string
template<> struct StringConv<QString, std::string> {
    static auto to(const QString& str) { return str.toStdString(); }
};

// QString -> std::u16string
template<> struct StringConv<QString, std::u16string> {
    static auto to(const QString& str) { return str.toStdU16String(); }
};

// QString -> std::u16string_view
template<> struct StringConv<QString, std::u16string_view> {
    static auto to(const QString& str) {
        return std::u16string_view(reinterpret_cast<const char16_t*>(str.utf16()), str.length());
    }
};

// QString -> TCollection_ExtendedString
template<> struct StringConv<QString, TCollection_ExtendedString> {
    static auto to(const QString& str) {
        return TCollection_ExtendedString(reinterpret_cast<const char16_t*>(str.utf16()));
    }
};

} // namespace Mayo
