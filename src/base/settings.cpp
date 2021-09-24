/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "settings.h"

#include "qtcore_utils.h"
#include "string_conv.h"

#include <QtCore/QtDebug>
#include <QtCore/QSettings>
#include <gsl/util>
#include <regex>

namespace Mayo {

namespace {

struct Settings_Setting {
    Property* property;
};

struct Settings_Section {
    TextId identifier; // Must be unique in the context of the parent group
    std::string overridenTitle;
    bool isDefault; // Default section in parent group
    std::vector<Settings_Setting> vecSetting;
};

struct Settings_Group {
    TextId identifier; // Must be unique in the context of the parent Settings object
    std::string overridenTitle;
    std::vector<Settings_Section> vecSection;
};

struct SectionResetFunction {
    Settings_SectionIndex sectionId;
    Settings::ResetFunction fnReset;
};

static bool isValidIdentifier(std::string_view identifier)
{
    return !identifier.empty() /*&& !identifier.simplified().isEmpty()*/;
}

} // namespace

class Settings::Private {
public:
    Private()
        : m_locale(QLocale::system()),
          m_propValueConverter(&m_defaultPropValueConverter)
    {
    }

    Settings_Group& group(Settings::GroupIndex index) {
        return m_vecGroup.at(index.get());
    }

    Settings_Section& section(Settings::SectionIndex index) {
        return this->group(index.group()).vecSection.at(index.get());
    }

    QString sectionPath(const Settings_Group& group, const Settings_Section& section) const {
        return to_QString(group.identifier.key) + "/" + to_QString(section.identifier.key);
    }

    QString sectionPath(Settings::SectionIndex index) {
        return this->sectionPath(this->group(index.group()), this->section(index));
    }

    void loadPropertyFrom(const QSettings& source, const QString& sectionPath, Property* property)
    {
        if (!property)
            return;

        std::string_view propertyKey = property->name().key;
        const QString settingPath = sectionPath + "/" + to_QString(propertyKey);
        if (source.contains(settingPath)) {
            const QVariant value = source.value(settingPath);
            const bool ok = m_propValueConverter->fromVariant(property, value);
            if (!ok)
                qCritical() << QString("Failed to load setting %1").arg(to_QString(propertyKey));
        }
    }

    QSettings m_settings;
    QLocale m_locale;
    std::vector<Settings_Group> m_vecGroup;
    std::vector<SectionResetFunction> m_vecSectionResetFn;
    const PropertyValueConversion m_defaultPropValueConverter;
    const PropertyValueConversion* m_propValueConverter = nullptr;
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

void Settings::loadFrom(const QSettings& source, const ExcludePropertyPredicate& fnExclude)
{
    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            const QString sectionPath = d->sectionPath(group, section);
            for (const Settings_Setting& setting : section.vecSetting) {
                if (!fnExclude || !fnExclude(*setting.property))
                    d->loadPropertyFrom(source, sectionPath, setting.property);
            }
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
        d->loadPropertyFrom(source, sectionPath, prop);
    }
}

QVariant Settings::findValueFromKey(std::string_view strKey) const
{
    return d->m_settings.value(to_QString(strKey));
}

void Settings::save()
{
    this->saveAs(&d->m_settings);
    d->m_settings.sync();
}

void Settings::saveAs(QSettings* target, const ExcludePropertyPredicate& fnExclude)
{
    if (!target)
        return;

    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            const QString sectionPath = d->sectionPath(group, section);
            for (const Settings_Setting& setting : section.vecSetting) {
                Property* prop = setting.property;
                if (!fnExclude || !fnExclude(*prop)) {
                    std::string_view propKey = prop->name().key;
                    const QString settingPath = sectionPath + "/" + to_QString(propKey);
                    target->setValue(settingPath, d->m_propValueConverter->toVariant(*prop));
                }
            } // endfor(settings)
        } // endfor(sections)
    } // endfor(groups)
}

const PropertyValueConversion& Settings::propertyValueConversion() const
{
    return *(d->m_propValueConverter);
}

void Settings::setPropertyValueConversion(const PropertyValueConversion& conv)
{
    d->m_propValueConverter = &conv;
}

int Settings::groupCount() const
{
    return int(d->m_vecGroup.size());
}

std::string_view Settings::groupIdentifier(GroupIndex index) const
{
    return d->group(index).identifier.key;
}

std::string_view Settings::groupTitle(GroupIndex index) const
{
    const Settings_Group& group = d->group(index);
    return !group.overridenTitle.empty() ? group.overridenTitle : group.identifier.tr();
}

Settings::GroupIndex Settings::addGroup(TextId identifier)
{
    auto index = this->addGroup(identifier.key);
    d->group(index).identifier = identifier;
    return index;
}

Settings::GroupIndex Settings::addGroup(std::string_view identifier)
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

void Settings::setGroupTitle(GroupIndex index, std::string_view title)
{
    d->group(index).overridenTitle = title;
}

void Settings::addResetFunction(GroupIndex index, Settings::ResetFunction fn)
{
    this->addResetFunction(SectionIndex(index, 0), std::move(fn));
}

void Settings::addResetFunction(SectionIndex index, ResetFunction fn)
{
    if (fn) {
        SectionResetFunction obj;
        obj.sectionId = index;
        obj.fnReset = std::move(fn);
        d->m_vecSectionResetFn.push_back(std::move(obj));
    }
}

int Settings::sectionCount(GroupIndex index) const
{
    return int(d->group(index).vecSection.size());
}

std::string_view Settings::sectionIdentifier(SectionIndex index) const
{
    return d->section(index).identifier.key;
}

std::string_view Settings::sectionTitle(SectionIndex index) const
{
    const Settings_Section& section = d->section(index);
    return !section.overridenTitle.empty() ? section.overridenTitle : section.identifier.tr();
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

Settings::SectionIndex Settings::addSection(GroupIndex index, std::string_view identifier)
{
    Expects(isValidIdentifier(identifier));
    // TODO Check identifier is unique

    Settings_Group& group = d->group(index);
    group.vecSection.push_back({});
    Settings_Section& section = group.vecSection.back();
    section.identifier.key = identifier;
    return SectionIndex(index, int(group.vecSection.size()) - 1);
}

void Settings::setSectionTitle(SectionIndex index, std::string_view title)
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
        const SectionIndex sectionId = this->addSection(groupId, { "Mayo::Settings", "DEFAULT" });
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

void Settings::resetAll()
{
    for (const SectionResetFunction& sectionResetFn : d->m_vecSectionResetFn)
        sectionResetFn.fnReset();
}

void Settings::resetGroup(GroupIndex index)
{
    for (const SectionResetFunction& sectionResetFn : d->m_vecSectionResetFn) {
        if (sectionResetFn.sectionId.group() == index)
            sectionResetFn.fnReset();
    }
}

void Settings::resetSection(SectionIndex index)
{
    for (const SectionResetFunction& sectionResetFn : d->m_vecSectionResetFn) {
        if (sectionResetFn.sectionId == index)
            sectionResetFn.fnReset();
    }
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

void Settings::onPropertyAboutToChange(Property* prop)
{
    PropertyGroup::onPropertyAboutToChange(prop);
    emit this->aboutToChange(prop);
}

void Settings::onPropertyChanged(Property* prop)
{
    PropertyGroup::onPropertyChanged(prop);
    emit this->changed(prop);
}

void Settings::onPropertyEnabled(Property* prop, bool on)
{
    PropertyGroup::onPropertyEnabled(prop, on);
    emit this->enabled(prop, on);
}

} // namespace Mayo
