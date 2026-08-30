/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/property_builtins.h"
#include "../src/base/settings.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace Mayo {

namespace {

class TestProperties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::TestProperties)
public:
    TestProperties(Settings* settings)
        : PropertyGroup(settings),
        groupId_main(settings->addGroup(textId("main")))
    {
        settings->addSetting(&this->someInt, groupId_main);
        settings->addResetFunction(groupId_main, [&]{
            this->someInt.setValue(-1);
        });
    }

    const Settings::GroupIndex groupId_main;
    PropertyInt someInt{ this, textId("someInt") };
};

class TestSettingsStorage : public Settings::Storage {
public:
    bool contains(std::string_view key) const override
    {
        return m_mapValue.find(key) != m_mapValue.cend();
    }

    Settings::Variant value(std::string_view key) const override
    {
        auto it = m_mapValue.find(key);
        return it != m_mapValue.cend() ? it->second : Settings::Variant{};
    }

    void setValue(std::string_view key, const Settings::Variant& value) override
    {
        m_mapValue.insert_or_assign(key, value);
    }

    void sync() override
    {
    }

private:
    std::unordered_map<std::string_view, Settings::Variant> m_mapValue;
};

} // namespace

void TestBase::Settings_test()
{
    Settings settings;
    {
        auto settingsStorage = std::make_unique<TestSettingsStorage>();
        settingsStorage->setValue("main/someInt", Settings::Variant{5});

        const uint8_t bytes[] = { 97, 98, 99, 100, 101, 95, 49, 50, 51, 52, 53 };
        const Settings::Variant bytesVar(gsl::span<const uint8_t>(bytes, std::size(bytes)));
        QVERIFY(std::holds_alternative<std::vector<uint8_t>>(bytesVar));
        settingsStorage->setValue("main/someTestData", bytesVar);

        settings.setStorage(std::move(settingsStorage));
    }

    TestProperties props(&settings);

    settings.resetAll();
    QCOMPARE(props.someInt.value(), -1);

    settings.load();
    QCOMPARE(props.someInt.value(), 5);
}

} // namespace Mayo
