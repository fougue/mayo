/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "settings.h"

namespace Mayo {

Settings* Settings::instance()
{
    static Settings settings;
    return &settings;
}

const QLocale& Settings::locale() const
{
    return m_locale;
}

void Settings::setLocale(const QLocale& locale)
{
    m_locale = locale;
}

QVariant Settings::value(const QString& key) const
{
    return m_settings.value(key, this->defaultValue(key));
}

void Settings::setValue(const QString& key, const QVariant& value)
{
    const QVariant oldValue = m_settings.value(key);
    if (value != oldValue) {
        m_settings.setValue(key, value);
        emit valueChanged(key, value);
    }
}

const QVariant& Settings::defaultValue(const QString& key) const
{
    static const QVariant nullVar;
    auto it = m_mapDefaultValue.find(key);
    return it != m_mapDefaultValue.cend() ? it->second : nullVar;
}

void Settings::setDefaultValue(const QString& key, const QVariant& value)
{
    auto it = m_mapDefaultValue.find(key);
    if (it != m_mapDefaultValue.end())
        it->second = value;
    else
        m_mapDefaultValue.insert({ key, value });
}

UnitSystem::Schema Settings::unitSystemSchema() const
{
    return this->valueAsEnum<UnitSystem::Schema>("Base/UnitSystemSchema");
}

int Settings::unitSystemDecimals() const
{
    return this->valueAs<int>("Base/UnitSystemDecimals");
}

StringUtils::TextOptions Settings::defaultTextOptions() const
{
    StringUtils::TextOptions opts;
    opts.locale = this->locale();
    opts.unitDecimals = this->unitSystemDecimals();
    opts.unitSchema = this->unitSystemSchema();
    return opts;
}

Settings::Settings()
    : m_locale(QLocale::system())
{
}

} // namespace Mayo
