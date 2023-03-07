/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "property_value_conversion.h"
#include "settings_index.h"
#include "signal.h"

#include <functional>
#include <memory>
#include <string_view>

namespace Mayo {

class Settings : public PropertyGroup {
public:
    using Variant = PropertyValueConversion::Variant;

    class Storage {
    public:
        virtual ~Storage() = default;
        virtual bool contains(std::string_view key) const = 0;
        virtual Variant value(std::string_view key) const = 0;
        virtual void setValue(std::string_view key, const Variant& value) = 0;
        virtual void sync() = 0;
    };

    using GroupIndex = Settings_GroupIndex;
    using SectionIndex = Settings_SectionIndex;
    using SettingIndex = Settings_SettingIndex;
    using ExcludePropertyPredicate = std::function<bool(const Property&)>;
    using ResetFunction = std::function<void()>;

    Settings();
    Settings(const Settings&) = delete; // Not copyable
    Settings& operator=(const Settings&) = delete; // Not copyable
    ~Settings();

    void setStorage(std::unique_ptr<Storage> ptrStorage);

    void load();
    void loadProperty(SettingIndex index);
    void loadProperty(const Property* property);
    Variant findValueFromKey(std::string_view strKey) const;
    void save();

    void loadPropertyFrom(const Storage& source, SettingIndex index);
    void loadFrom(const Storage& source, const ExcludePropertyPredicate& fnExclude = nullptr);
    void saveAs(Storage* target, const ExcludePropertyPredicate& fnExclude = nullptr);

    const PropertyValueConversion& propertyValueConversion() const;
    void setPropertyValueConversion(const PropertyValueConversion* conv);

    int groupCount() const;
    std::string_view groupIdentifier(GroupIndex index) const;
    std::string_view groupTitle(GroupIndex index) const;
    GroupIndex addGroup(TextId identifier);
    GroupIndex addGroup(std::string_view identifier);
    void setGroupTitle(GroupIndex index, std::string_view title);

    void addResetFunction(GroupIndex index, ResetFunction fn);
    void addResetFunction(SectionIndex index, ResetFunction fn);

    int sectionCount(GroupIndex index) const;
    std::string_view sectionIdentifier(SectionIndex index) const;
    std::string_view sectionTitle(SectionIndex index) const;
    bool isDefaultGroupSection(SectionIndex index) const;
    SectionIndex addSection(GroupIndex index, TextId identifier);
    SectionIndex addSection(GroupIndex index, std::string_view identifier);
    void setSectionTitle(SectionIndex index, std::string_view title);

    int settingCount(SectionIndex index) const;
    Property* property(SettingIndex index) const;
    SettingIndex findProperty(const Property* property) const;
    SettingIndex addSetting(Property* property, GroupIndex index);
    SettingIndex addSetting(Property* property, SectionIndex index);

    void resetAll();
    void resetGroup(GroupIndex index);
    void resetSection(SectionIndex index);

    // Signals
    Signal<Property*> signalAboutToChange;
    Signal<Property*> signalChanged;
    Signal<Property*, bool> signalEnabled;

protected:
    void onPropertyAboutToChange(Property* prop) override;
    void onPropertyChanged(Property* prop) override;
    void onPropertyEnabled(Property* prop, bool on) override;

private:
    class Private;
    Private* const d = nullptr;
};

} // namespace Mayo
