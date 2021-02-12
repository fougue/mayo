/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "unit_system.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>
class Quantity_Color;
class gp_Pnt;
class gp_Dir;
class gp_Trsf;

namespace Mayo {

class StringUtils {
    Q_DECLARE_TR_FUNCTIONS(Mayo::StringUtils)
public:
    struct TextOptions {
        QLocale locale;
        UnitSystem::Schema unitSchema;
        int unitDecimals;
    };

    // TODO add overload for 'int' type
    static QString text(double value, const TextOptions& opt);
    static QString text(const gp_Pnt& pos, const TextOptions& opt);
    static QString text(const gp_Dir& pos, const TextOptions& opt);
    static QString text(const gp_Trsf& trsf, const TextOptions& opt);
    static QString text(const Quantity_Color& color, const QString& format = "RGB(%1, %2 %3)");

    static QString bytesText(uint64_t sizeBytes, const QLocale& locale = QLocale());

    static QString yesNoText(bool on);
    static QString yesNoText(Qt::CheckState state);

    static void append(QString* dst, const QString& str, const QLocale& locale = QLocale());

    // Qt/OpenCascade string conversion
    template<typename OTHER_STRING_TYPE>
    static OTHER_STRING_TYPE toUtf8(const QString& str);

    template<typename OTHER_STRING_TYPE>
    static OTHER_STRING_TYPE toUtf16(const QString& str);

    static QString fromUtf8(const TCollection_AsciiString& str);
    static QString fromUtf8(const Handle_TCollection_HAsciiString& str);
    static QString fromUtf8(std::string_view str);
    static QString fromUtf16(const TCollection_ExtendedString& str);
    // TODO Add conversion for NCollection_String
};

// --
// -- Implementation
// --

template<typename OTHER_STRING_TYPE>
OTHER_STRING_TYPE StringUtils::toUtf8(const QString& str) {
    if constexpr(std::is_same<OTHER_STRING_TYPE, TCollection_AsciiString>::value) {
        return TCollection_AsciiString(qUtf8Printable(str));
    }
    else if constexpr(std::is_same<OTHER_STRING_TYPE, Handle_TCollection_HAsciiString>::value) {
        Handle_TCollection_HAsciiString hStr = new TCollection_HAsciiString(qUtf8Printable(str));
        return hStr;
    }
    else if constexpr(std::is_same<OTHER_STRING_TYPE, std::string>::value) {
        return str.toStdString();
    }
}

template<typename OTHER_STRING_TYPE>
OTHER_STRING_TYPE StringUtils::toUtf16(const QString& str) {
    if constexpr(std::is_same<OTHER_STRING_TYPE, TCollection_ExtendedString>::value) {
        return TCollection_ExtendedString(reinterpret_cast<Standard_ExtString>(str.utf16()));
    }
    else if constexpr(std::is_same<OTHER_STRING_TYPE, Standard_ExtString>::value) {
        return reinterpret_cast<Standard_ExtString>(str.utf16());
    }
    else if constexpr(std::is_same<OTHER_STRING_TYPE, std::wstring>::value) {
        return str.toStdWString();
    }
}

} // namespace Mayo
