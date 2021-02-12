/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "settings.h"

#include <QtCore/QSettings>
#include <gsl/gsl_util>
#include <regex>

namespace Mayo {

namespace {

struct Settings_Setting {
    Property* property;
};

struct Settings_Section {
    TextId identifier; // Must be unique in the context of the parent group
    QString overridenTitle;
    bool isDefault; // Default section in parent group
    std::vector<Settings_Setting> vecSetting;
};

struct Settings_Group {
    TextId identifier; // Must be unique in the context of the parent Settings object
    QString overridenTitle;
    std::vector<Settings_Section> vecSection;
    std::vector<Settings::GroupResetFunction> vecFnReset;
};

static bool isValidIdentifier(const QByteArray& identifier)
{
    return !identifier.isEmpty() && !identifier.simplified().isEmpty();
}

} // namespace

class Settings::Private {
public:
    Private()
        : m_locale(QLocale::system())
    {}

    Settings_Group& group(Settings::GroupIndex index) {
        return m_vecGroup.at(index.get());
    }

    Settings_Section& section(Settings::SectionIndex index) {
        return this->group(index.group()).vecSection.at(index.get());
    }

    QString sectionPath(const Settings_Group& group, const Settings_Section& section) const {
        return QString::fromUtf8(group.identifier.key) + "/" + QString::fromUtf8(section.identifier.key);
    }

    QString sectionPath(Settings::SectionIndex index) {
        return this->sectionPath(this->group(index.group()), this->section(index));
    }

    static void loadPropertyFrom(const QSettings& source, const QString& sectionPath, Property* property)
    {
        if (!property)
            return;

        const QByteArray propertyKey = property->name().key;
        const QString settingPath = sectionPath + "/" + QString::fromUtf8(propertyKey);
        if (source.contains(settingPath)) {
            const QVariant value = source.value(settingPath);
            property->setValueFromVariant(value);
        }
    }

    QSettings m_settings;
    QLocale m_locale;
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

void Settings::load()
{
    this->loadFrom(d->m_settings);
}

void Settings::loadFrom(const QSettings& source)
{
    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            const QString sectionPath = d->sectionPath(group, section);
            for (const Settings_Setting& setting : section.vecSetting)
                Private::loadPropertyFrom(source, sectionPath, setting.property);
        }
    }
}

void Settings::loadProperty(Settings::SettingIndex index)
{
    this->loadPropertyFrom(d->m_settings, index);
}

void Settings::loadPropertyFrom(const QSettings& source, SettingIndex index)
{
    Property* prop = this->property(index);
    if (prop) {
        const QString sectionPath = d->sectionPath(index.section());
        Private::loadPropertyFrom(source, sectionPath, prop);
    }
}

QVariant Settings::findValueFromKey(const QString& strKey) const
{
    return d->m_settings.value(strKey);
}

void Settings::save()
{
    this->saveAs(&d->m_settings);
    d->m_settings.sync();
}

void Settings::saveAs(QSettings* target)
{
    if (!target)
        return;

    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            const QString sectionPath = d->sectionPath(group, section);
            for (const Settings_Setting& setting : section.vecSetting) {
                Property* prop = setting.property;
                const QByteArray propKey = prop->name().key;
                const QString settingPath = sectionPath + "/" + QString::fromUtf8(propKey);
                target->setValue(settingPath, prop->valueAsVariant());
            } // endfor(settings)
        } // endfor(sections)
    } // endfor(groups)
}

int Settings::groupCount() const
{
    return int(d->m_vecGroup.size());
}

QByteArray Settings::groupIdentifier(GroupIndex index) const
{
    return d->group(index).identifier.key;
}

QString Settings::groupTitle(GroupIndex index) const
{
    const Settings_Group& group = d->group(index);
    return !group.overridenTitle.isEmpty() ? group.overridenTitle : group.identifier.tr();
}

Settings::GroupIndex Settings::addGroup(TextId identifier)
{
    auto index = this->addGroup(identifier.key);
    d->group(index).identifier = identifier;
    return index;
}

Settings::GroupIndex Settings::addGroup(QByteArray identifier)
{
    Expects(isValidIdentifier(identifier));

    for (const Settings_Group& group : d->m_vecGroup) {
        if (group.identifier.key == identifier)
            return GroupIndex(&group - &d->m_vecGroup.front());
    }

    d->m_vecGroup.push_back({});
    Settings_Group& group = d->m_vecGroup.back();
    group.identifier.key = identifier;

    Settings_Section defaultSection;
    defaultSection.isDefault = true;
    group.vecSection.push_back(std::move(defaultSection));

    return GroupIndex(int(d->m_vecGroup.size()) - 1);
}

void Settings::setGroupTitle(GroupIndex index, const QString& title)
{
    d->group(index).overridenTitle = title;
}

void Settings::addGroupResetFunction(GroupIndex index, GroupResetFunction fn)
{
    if (fn)
        d->group(index).vecFnReset.push_back(std::move(fn));
}

int Settings::sectionCount(GroupIndex index) const
{
    return int(d->group(index).vecSection.size());
}

QByteArray Settings::sectionIdentifier(SectionIndex index) const
{
    return d->section(index).identifier.key;
}

QString Settings::sectionTitle(SectionIndex index) const
{
    const Settings_Section& section = d->section(index);
    return !section.overridenTitle.isEmpty() ? section.overridenTitle : section.identifier.tr();
}

bool Settings::isDefaultGroupSection(SectionIndex index) const
{
    return d->section(index).isDefault;
}

Settings::SectionIndex Settings::addSection(GroupIndex index, TextId identifier)
{
    auto sectionIndex = this->addSection(index, identifier.key);
    d->section(sectionIndex).identifier = identifier;
    return sectionIndex;
}

Settings::SectionIndex Settings::addSection(GroupIndex index, QByteArray identifier)
{
    Expects(isValidIdentifier(identifier));
    // TODO Check identifier is unique

    Settings_Group& group = d->group(index);
    group.vecSection.push_back({});
    Settings_Section& section = group.vecSection.back();
    section.identifier.key = identifier;
    return SectionIndex(index, int(group.vecSection.size()) - 1);
}

void Settings::setSectionTitle(SectionIndex index, const QString& title)
{
    d->section(index).overridenTitle = title;
}

int Settings::settingCount(SectionIndex index) const
{
    return int(d->section(index).vecSetting.size());
}

Property* Settings::property(SettingIndex index) const
{
    return d->section(index.section()).vecSetting.at(index.get()).property;
}

Settings::SettingIndex Settings::findProperty(const Property* property) const
{
    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            for (const Settings_Setting& setting : section.vecSetting) {
                if (setting.property == property) {
                    const int idSetting = &setting - &section.vecSetting.front();
                    const int idSection = &section - &group.vecSection.front();
                    const int idGroup = &group - &d->m_vecGroup.front();
                    return SettingIndex(SectionIndex(GroupIndex(idGroup), idSection), idSetting);
                }
            }
        }
    }

    return {};
}

Settings::SettingIndex Settings::addSetting(Property* property, GroupIndex groupId)
{
    Settings_Group& group = d->group(groupId);
    Settings_Section* sectionDefault = nullptr;
    if (group.vecSection.empty()) {
        const SectionIndex sectionId = this->addSection(groupId, MAYO_TEXT_ID("Mayo::Settings", "DEFAULT"));
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

//    sectionDefault->identifier = "DEFAULT";
//    sectionDefault->title = tr("DEFAULT");
    sectionDefault->isDefault = true;
    const SectionIndex sectionId(groupId, sectionDefault - &group.vecSection.front());
    return this->addSetting(property, sectionId);
}

Settings::SettingIndex Settings::addSetting(Property* property, SectionIndex index)
{
    // TODO Check identifier is unique
    Settings_Section& section = d->section(index);
    section.vecSetting.push_back({});
    Settings_Setting& setting = section.vecSetting.back();
    setting.property = property;
    return SettingIndex(index, int(section.vecSetting.size()) - 1);
}

void Settings::resetGroup(GroupIndex index)
{
    Settings_Group& group = d->group(index);
    for (const GroupResetFunction& fnReset : group.vecFnReset)
        fnReset();
}

void Settings::resetAll()
{
    for (Settings_Group& group : d->m_vecGroup)
        this->resetGroup(GroupIndex(&group - &d->m_vecGroup.front()));
}

QByteArray Settings::defautLocaleLanguageCode()
{
    const QByteArray localeName = QLocale().name().toUtf8();
    const std::regex rxLang("([a-z]+)_");
    std::cmatch rxLangMatch;
    if (std::regex_match(localeName.cbegin(), localeName.cend(), rxLangMatch, rxLang))
        return QByteArray::fromStdString(rxLangMatch.str(1));

    return QByteArrayLiteral("en");
}

const QLocale& Settings::locale() const
{
    return d->m_locale;
}

void Settings::setLocale(const QLocale& locale)
{
    d->m_locale = locale;
}

void Settings::onPropertyChanged(Property* prop)
{
    PropertyGroup::onPropertyChanged(prop);
    emit this->changed(prop);
}

} // namespace Mayo
