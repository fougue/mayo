 /****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "settings.h"

#include <fmt/format.h>
#include <gsl/util>
#include <iostream>
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

class VoidStorage : public Settings::Storage {
public:
    bool contains(std::string_view /*key*/) const override { return false; }
    Settings::Variant value(std::string_view /*key*/) const override { return {}; }
    void setValue(std::string_view /*key*/, const Settings::Variant& /*value*/) override {}
    void sync() override {}
};

} // namespace

class Settings::Private {
public:
    Private()
        : m_storage(new VoidStorage),
          m_propValueConverter(&m_defaultPropValueConverter)
    {
    }

    Settings_Group& group(Settings::GroupIndex index) {
        return m_vecGroup.at(index.get());
    }

    Settings_Section& section(Settings::SectionIndex index) {
        return this->group(index.group()).vecSection.at(index.get());
    }

    std::string sectionPath(const Settings_Group& group, const Settings_Section& section) const {
        std::string path(group.identifier.key);
        if (!path.empty() && path.back() != '/' && !section.identifier.key.empty())
            path += '/';

        return path.append(section.identifier.key);
    }

    std::string sectionPath(Settings::SectionIndex index) {
        return this->sectionPath(this->group(index.group()), this->section(index));
    }

    void loadPropertyFrom(const Settings::Storage& source, std::string_view sectionPath, Property* property)
    {
        if (!property)
            return;

        std::string_view propertyKey = property->name().key;
        const std::string settingPath = std::string(sectionPath).append("/").append(propertyKey);
        if (source.contains(settingPath)) {
            const Settings::Variant value = source.value(settingPath);
            const bool ok = m_propValueConverter->fromVariant(property, value);               
            if (!ok) {
                // TODO Use other output stream(dedicated Messenger object?)
                std::cerr << fmt::format("Failed to load setting [path={}]", settingPath) << std::endl;
            }
        }
    }

    Settings::Storage& storage() { return *m_storage.get(); }

    std::unique_ptr<Settings::Storage> m_storage;
    std::vector<Settings_Group> m_vecGroup;
    std::vector<SectionResetFunction> m_vecSectionResetFn;
    const PropertyValueConversion m_defaultPropValueConverter;
    const PropertyValueConversion* m_propValueConverter = nullptr;
};

Settings::Settings()
    : d(new Private)
{
}

Settings::~Settings()
{
    delete d;
}

void Settings::setStorage(std::unique_ptr<Settings::Storage> ptrStorage)
{
    if (ptrStorage)
        d->m_storage = std::move(ptrStorage);
    else
        d->m_storage.reset(new VoidStorage);
}

void Settings::load()
{
    this->loadFrom(d->storage());
}

void Settings::loadFrom(const Storage& source, const ExcludePropertyPredicate& fnExclude)
{
    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            const std::string sectionPath = d->sectionPath(group, section);
            for (const Settings_Setting& setting : section.vecSetting) {
                if (!fnExclude || !fnExclude(*setting.property))
                    d->loadPropertyFrom(source, sectionPath, setting.property);
            }
        }
    }
}

void Settings::loadProperty(Settings::SettingIndex index)
{
    this->loadPropertyFrom(d->storage(), index);
}

void Settings::loadProperty(const Property* property)
{
    const auto idProp = this->findProperty(property);
    this->loadPropertyFrom(d->storage(), idProp);
}

void Settings::loadPropertyFrom(const Storage& source, SettingIndex index)
{
    Property* prop = this->property(index);
    if (prop) {
        const std::string sectionPath = d->sectionPath(index.section());
        d->loadPropertyFrom(source, sectionPath, prop);
    }
}

Settings::Variant Settings::findValueFromKey(std::string_view strKey) const
{
    return d->m_storage->value(strKey);
}

void Settings::save()
{
    this->saveAs(d->m_storage.get());
    d->m_storage->sync();
}

void Settings::saveAs(Storage* target, const ExcludePropertyPredicate& fnExclude)
{
    if (!target)
        return;

    for (const Settings_Group& group : d->m_vecGroup) {
        for (const Settings_Section& section : group.vecSection) {
            const std::string sectionPath = d->sectionPath(group, section);
            for (const Settings_Setting& setting : section.vecSetting) {
                Property* prop = setting.property;
                if (!fnExclude || !fnExclude(*prop)) {
                    std::string_view propKey = prop->name().key;
                    const std::string settingPath = std::string(sectionPath).append("/").append(propKey);
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

void Settings::setPropertyValueConversion(const PropertyValueConversion* conv)
{
    d->m_propValueConverter = conv;
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
            return GroupIndex(Span_itemIndex(d->m_vecGroup, group));
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
                    const auto idSetting = Span_itemIndex(section.vecSetting, setting);
                    const auto idSection = Span_itemIndex(group.vecSection, section);
                    const auto idGroup = Span_itemIndex(d->m_vecGroup, group);
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
    const SectionIndex sectionId(groupId, Span_itemIndex(group.vecSection, *sectionDefault));
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

void Settings::onPropertyAboutToChange(Property* prop)
{
    PropertyGroup::onPropertyAboutToChange(prop);
    this->signalAboutToChange.send(prop);
}

void Settings::onPropertyChanged(Property* prop)
{
    PropertyGroup::onPropertyChanged(prop);
    this->signalChanged.send(prop);
}

void Settings::onPropertyEnabled(Property* prop, bool on)
{
    PropertyGroup::onPropertyEnabled(prop, on);
    this->signalEnabled.send(prop, on);
}

} // namespace Mayo
