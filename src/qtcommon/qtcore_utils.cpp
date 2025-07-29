/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qtcore_utils.h"

#include <algorithm>

#include <QtCore/QCoreApplication>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

namespace Mayo::QtCoreUtils {

QByteArray QByteArray_fromRawData(const QByteArray& bytes)
{
    return QByteArray::fromRawData(bytes.data(), bytes.size());
}

QByteArray QByteArray_fromRawData(std::string_view str)
{
    return QByteArray::fromRawData(str.data(), int(str.size()));
}

std::vector<uint8_t> toStdByteArray(const QByteArray& bytes)
{
    std::vector<uint8_t> stdBytes;
    stdBytes.resize(bytes.size());
    std::copy(bytes.cbegin(), bytes.cend(), stdBytes.begin());
    return stdBytes;
}

Qt::CheckState toQtCheckState(Mayo::CheckState state)
{
    switch (state) {
    case CheckState::Off: return Qt::Unchecked;
    case CheckState::Partially: return Qt::PartiallyChecked;
    case CheckState::On: return Qt::Checked;
    }

    return Qt::Unchecked;
}

Mayo::CheckState toCheckState(Qt::CheckState state)
{
    switch (state) {
    case Qt::Unchecked: return CheckState::Off;
    case Qt::PartiallyChecked: return CheckState::Partially;
    case Qt::Checked: return CheckState::On;
    }

    return CheckState::Off;
}

PropertyValueConversion::Variant toPropertyValueConversionVariant(const QVariant& value)
{
    switch (value.type()) {
    case QVariant::ByteArray: {
        auto bytes = QtCoreUtils::toStdByteArray(value.toByteArray());
        return PropertyValueConversion::Variant(bytes);
    }
    case QVariant::String: {
        const QString strval = value.toString();
        if (strval == "true")
            return true;
        else if (strval == "false")
            return false;

        bool ok = false;
        const int ival = strval.toInt(&ok);
        if (ok)
            return ival;

        const double dval = strval.toDouble(&ok);
        if (ok)
            return dval;

        return strval.toStdString();
    }
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong: {
        bool ok = true;
        const int ivalue = value.toInt(&ok);
        return ok ? PropertyValueConversion::Variant(ivalue) : PropertyValueConversion::Variant{};
    }
    case QVariant::Double: {
        bool ok = true;
        const double dvalue = value.toDouble(&ok);
        return ok ? PropertyValueConversion::Variant(dvalue) : PropertyValueConversion::Variant{};
    }
    case QVariant::Bool:
        return value.toBool();
    default:
        return {};
    } // endswitch

    return {};
}

QVariant toQVariant(const PropertyValueConversion::Variant& value)
{
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    }
    else if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    }
    else if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    else if (std::holds_alternative<std::string>(value)) {
        return QString::fromStdString(value.toConstRefString());
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
        return QtCoreUtils::toQByteArray(value.toConstRefByteArray());
    }

    return {};
}

void runJobOnMainThread(const std::function<void()>& fn)
{
    QTimer::singleShot(0, QCoreApplication::instance(), fn);
}

} // namespace Mayo::QtCoreUtils
