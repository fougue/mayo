/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/string_conv.h"

#include <QtCore/QString>

namespace Mayo {

// General API

template<typename StringType>
QString to_QString(const StringType& str) {
    return string_conv<QString>(str);
}

// Returns 'str' converted to a "guaranteed" printable string
std::string consoleToPrintable(const QString& str);
std::string consoleToPrintable(std::string_view str/*utf8*/);

// --
// -- X -> QString
// --

// const char* -> QString
template<> struct StringConv<const char*, QString> {
    static auto to(const char* str) { return QString::fromUtf8(str); }
};

// const char[N] -> QString
template<size_t N> struct StringConv<char[N], QString> {
    static auto to(const char(&str)[N]) { return QString::fromUtf8(str, N); }
};

// std::string -> QString
template<> struct StringConv<std::string, QString> {
    static auto to(const std::string& str) { return QString::fromStdString(str); }
};

// std::wstring -> QString
template<> struct StringConv<std::wstring, QString> {
    static auto to(const std::wstring& str) { return QString::fromStdWString(str); }
};

// std::string_view -> QString
template<> struct StringConv<std::string_view, QString> {
    static auto to(std::string_view str) { return QString::fromUtf8(str.data(), int(str.size())); }
};

// QByteArray -> QString
template<> struct StringConv<QByteArray, QString> {
    static auto to(const QByteArray& str) { return QString::fromUtf8(str); }
};

// TCollection_AsciiString -> QString
template<> struct StringConv<TCollection_AsciiString, QString> {
    static auto to(const TCollection_AsciiString& str) {
        return QString::fromUtf8(str.ToCString(), str.Length());
    }
};

// Handle(TCollection_HAsciiString) -> QString
template<> struct StringConv<OccHandle<TCollection_HAsciiString>, QString> {
    static auto to(const OccHandle<TCollection_HAsciiString>& str) {
        return string_conv<QString>(str ? str->String() : TCollection_AsciiString());
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
// -- QString -> X
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

// QString -> std::wstring
template<> struct StringConv<QString, std::wstring> {
    static auto to(const QString& str) { return str.toStdWString(); }
};

// QString -> TCollection_ExtendedString
template<> struct StringConv<QString, TCollection_ExtendedString> {
    static auto to(const QString& str) {
        return TCollection_ExtendedString(reinterpret_cast<const char16_t*>(str.utf16()));
    }
};

// QString -> TCollection_AsciiString
template<> struct StringConv<QString, TCollection_AsciiString> {
    static auto to(const QString& str) {
        return TCollection_AsciiString(qUtf8Printable(str));
    }
};

// QString -> Handle(TCollection_HAsciiString)
template<> struct StringConv<QString, OccHandle<TCollection_HAsciiString>> {
    static auto to(const QString& str) {
        return makeOccHandle<TCollection_HAsciiString>(qUtf8Printable(str));
    }
};

// --
// -- Converters(misc)
// --

// TCollection_AsciiString -> QByteArray
template<> struct StringConv<TCollection_AsciiString, QByteArray> {
    static auto to(const TCollection_AsciiString& str) {
        return QByteArray(str.ToCString(), str.Length());
    }
};

// TCollection_AsciiString -> QLatin1String
template<> struct StringConv<TCollection_AsciiString, QLatin1String> {
    static auto to(const TCollection_AsciiString& str) {
        return QLatin1String(str.ToCString(), str.Length());
    }
};

} // namespace Mayo
