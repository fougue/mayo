/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/unit_system.h"
#include "../base/string_utils.h"
#include "settings_index.h"

#include <QtCore/QLocale>
#include <QtCore/QObject>

namespace Mayo {

class Property;

//class Module {
//public:
//    virtual void registerSettings() = 0;
//};

//class GuiModule {
//public:
//    static void init();
//};

class Settings : public QObject {
    Q_OBJECT
public:
    using GroupIndex = Settings_GroupIndex;
    using SectionIndex = Settings_SectionIndex;
    using SettingIndex = Settings_SettingIndex;

    static Settings* instance();
    ~Settings();

    int groupCount() const;
    QByteArray groupId(GroupIndex index) const;
    QString groupTitle(GroupIndex index) const;
    void setGroupTitle(GroupIndex index, const QString& title) const;
    GroupIndex addGroup(QByteArray identifier);

    int sectionCount(GroupIndex index) const;
    QByteArray sectionIdentifier(SectionIndex index) const;
    QString sectionTitle(SectionIndex index) const;
    void setSectionTitle(SectionIndex index, const QString& title) const;
    bool isDefaultGroupSection(SectionIndex index) const;
    SectionIndex addSection(GroupIndex index, QByteArray identifier);

    int settingCount(SectionIndex index) const;
    Property* property(SettingIndex index) const;
    SettingIndex addSetting(Property* property, GroupIndex index);
    SettingIndex addSetting(Property* property, SectionIndex index);

    // Helpers

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

    class Private;
    Private* const d = nullptr;
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
