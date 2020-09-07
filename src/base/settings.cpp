/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "settings.h"

#include <fougtools/qttools/core/qstring_hfunc.h>
#include <QtCore/QSettings>
#include <gsl/gsl_util>
#include <unordered_map>

namespace Mayo {

namespace {

struct Settings_Section {
    QByteArray id; // Must be unique in the context of the parent group
    QString title;
    bool isDefault; // Default section in parent group
    std::vector<Property*> vecSetting;
};

struct Settings_Group {
    QByteArray id; // Must be unique in the context of the parent Settings object
    QString title;
    std::vector<Settings_Section> vecSection;
    Settings_Section defaultSection;
};

static bool isValidIdentifier(const QByteArray& id)
{
    return !id.isEmpty() && !id.simplified().isEmpty();
}

} // namespace

class Settings::Private {
public:
    Private()
        : m_locale(QLocale::system())
    {}

    Settings_Group& group(GroupIndex index) { return m_vecGroup.at(index.get()); }
    Settings_Section& section(SectionIndex index) { return this->group(index.group()).vecSection.at(index.get()); }

    QSettings m_settings;
    QLocale m_locale;
    std::unordered_map<QString, QVariant> m_mapDefaultValue;
    std::vector<Settings_Group> m_vecGroup;
};

Settings::Settings(QObject* parent)
    : QObject(parent),
      d(new Private)
{
}

Settings::~Settings()
{
    delete d;
}

int Settings::groupCount() const
{
    return int(d->m_vecGroup.size());
}

QByteArray Settings::groupId(GroupIndex index) const
{
    return d->group(index).id;
}

QString Settings::groupTitle(GroupIndex index) const
{
    return d->group(index).title;
}

void Settings::setGroupTitle(GroupIndex index, const QString& title) const
{
    d->group(index).title = title;
}

Settings::GroupIndex Settings::addGroup(QByteArray identifier)
{
    Expects(isValidIdentifier(identifier));
    // TODO Check identifier is unique

    d->m_vecGroup.push_back({});
    Settings_Group& group = d->m_vecGroup.back();
    group.id = identifier;
    return GroupIndex(int(d->m_vecGroup.size()) - 1);
}

int Settings::sectionCount(GroupIndex index) const
{
    return int(d->group(index).vecSection.size());
}

QByteArray Settings::sectionIdentifier(SectionIndex index) const
{
    return d->section(index).id;
}

QString Settings::sectionTitle(SectionIndex index) const
{
    return d->section(index).title;
}

void Settings::setSectionTitle(SectionIndex index, const QString& title) const
{
    d->section(index).title = title;
}

bool Settings::isDefaultGroupSection(SectionIndex index) const
{
    return d->section(index).isDefault;
}

Settings::SectionIndex Settings::addSection(GroupIndex index, QByteArray identifier)
{
    Expects(isValidIdentifier(identifier));
    // TODO Check identifier is unique

    d->group(index).vecSection.push_back({});
    Settings_Section& section = d->group(index).vecSection.back();
    section.id = identifier;
    return SectionIndex(index, int(d->group(index).vecSection.size()) - 1);
}

int Settings::settingCount(SectionIndex index) const
{
    return int(d->section(index).vecSetting.size());
}

Property* Settings::property(SettingIndex index) const
{
    return d->section(index.section()).vecSetting.at(index.get());
}

Settings::SettingIndex Settings::addSetting(Property* property, GroupIndex groupId)
{
    Settings_Group& group = d->group(groupId);
    Settings_Section* sectionDefault = nullptr;
    if (group.vecSection.empty()) {
        const SectionIndex sectionId = this->addSection(groupId, "DEFAULT");
        sectionDefault = &d->section(sectionId);
    }
    else {
        if (group.vecSection.front().isDefault) {
            sectionDefault = &group.vecSection.front();
        }
        else {
            group.vecSection.insert(group.vecSection.begin(), {});
            sectionDefault = &group.vecSection.front();
        }
    }

    sectionDefault->id = "DEFAULT";
    sectionDefault->isDefault = true;
    return this->addSetting(property, SectionIndex(groupId, sectionDefault - &group.vecSection.front()));
}

Settings::SettingIndex Settings::addSetting(Property* property, SectionIndex index)
{
    Settings_Section& section = d->section(index);
    section.vecSetting.push_back(property);
    return SettingIndex(index, int(section.vecSetting.size()) - 1);
}

const QLocale& Settings::locale() const
{
    return d->m_locale;
}

void Settings::setLocale(const QLocale& locale)
{
    d->m_locale = locale;
}

QVariant Settings::value(const QString& key) const
{
    return d->m_settings.value(key, this->defaultValue(key));
}

void Settings::setValue(const QString& key, const QVariant& value)
{
    const QVariant oldValue = d->m_settings.value(key);
    if (value != oldValue) {
        d->m_settings.setValue(key, value);
        emit valueChanged(key, value);
    }
}

const QVariant& Settings::defaultValue(const QString& key) const
{
    static const QVariant nullVar;
    auto it = d->m_mapDefaultValue.find(key);
    return it != d->m_mapDefaultValue.cend() ? it->second : nullVar;
}

void Settings::setDefaultValue(const QString& key, const QVariant& value)
{
    auto it = d->m_mapDefaultValue.find(key);
    if (it != d->m_mapDefaultValue.end())
        it->second = value;
    else
        d->m_mapDefaultValue.insert({ key, value });
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

} // namespace Mayo
