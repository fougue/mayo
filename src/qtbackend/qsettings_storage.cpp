/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qsettings_storage.h"

#include "../qtcommon/qstring_conv.h"
#include "../qtcommon/qtcore_utils.h"

namespace Mayo {

QSettingsStorage::QSettingsStorage(const QString& fileName, QSettings::Format format)
    : m_storage(fileName, format)
{
}

bool QSettingsStorage::contains(std::string_view key) const
{
    return m_storage.contains(to_QString(key));
}

Settings::Variant QSettingsStorage::value(std::string_view key) const
{
    const QVariant value = m_storage.value(to_QString(key));
    switch (value.type()) {
    case QVariant::ByteArray: {
        auto bytes = QtCoreUtils::toStdByteArray(value.toByteArray());
        return Settings::Variant(bytes);
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

        return to_stdString(strval);
    }
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong: {
        bool ok = true;
        const int ivalue = value.toInt(&ok);
        return ok ? Settings::Variant(ivalue) : Settings::Variant{};
    }
    case QVariant::Double: {
        bool ok = true;
        const double dvalue = value.toDouble(&ok);
        return ok ? Settings::Variant(dvalue) : Settings::Variant{};
    }
    case QVariant::Bool:
        return value.toBool();
    default:
        return {};
    } // endswitch

    return {};
}

void QSettingsStorage::setValue(std::string_view key, const Settings::Variant& value)
{
    QVariant qvalue;
    if (std::holds_alternative<bool>(value)) {
        qvalue = std::get<bool>(value);
    }
    else if (std::holds_alternative<int>(value)) {
        qvalue = std::get<int>(value);
    }
    else if (std::holds_alternative<double>(value)) {
        qvalue = std::get<double>(value);
    }
    else if (std::holds_alternative<std::string>(value)) {
        qvalue = to_QString(value.toConstRefString());
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
        qvalue = QtCoreUtils::toQByteArray(value.toConstRefByteArray());
    }

    if (!qvalue.isNull())
        m_storage.setValue(to_QString(key), qvalue);
}

void QSettingsStorage::sync()
{
    m_storage.sync();
}

} // namespace Mayo
