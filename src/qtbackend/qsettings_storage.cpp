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
    return QtCoreUtils::toPropertyValueConversionVariant(value);
}

void QSettingsStorage::setValue(std::string_view key, const Settings::Variant& value)
{
    const QVariant qvalue = QtCoreUtils::toQVariant(value);
    if (!qvalue.isNull())
        m_storage.setValue(to_QString(key), qvalue);
}

void QSettingsStorage::sync()
{
    m_storage.sync();
}

} // namespace Mayo
