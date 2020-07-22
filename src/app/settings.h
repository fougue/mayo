/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <fougtools/qttools/core/qstring_hfunc.h>
#include <QtCore/QLocale>
#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <unordered_map>

#include "../base/unit_system.h"
#include "../base/string_utils.h"

namespace Mayo {

class Settings : public QObject {
    Q_OBJECT
public:
    static Settings* instance();

    const QLocale& locale() const;
    void setLocale(const QLocale& locale);

    QVariant value(const QString& key) const;
    template<typename T> T valueAs(const QString& key) const;
    template<typename ENUM> ENUM valueAsEnum(const QString& key) const;
    void setValue(const QString& key, const QVariant& value);

    const QVariant& defaultValue(const QString& key) const;
    void setDefaultValue(const QString& key, const QVariant& value);

    UnitSystem::Schema unitSystemSchema() const;
    int unitSystemDecimals() const;
    StringUtils::TextOptions defaultTextOptions() const;

signals:
    void valueChanged(const QString& key, const QVariant& value);

private:
    Settings();

    QSettings m_settings;
    QLocale m_locale;
    std::unordered_map<QString, QVariant> m_mapDefaultValue;
};



// --
// -- Implementation
// --

template<typename ENUM> ENUM Settings::valueAsEnum(const QString& key) const {
    // TODO Check returned value is QVariant-convertible to 'int'
    return static_cast<ENUM>(this->value(key).toInt());
}

template<typename T> T Settings::valueAs(const QString& key) const {
    // TODO Check returned value is QVariant-convertible to 'T'
    return this->value(key).value<T>();
}

} // namespace Mayo
