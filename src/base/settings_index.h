/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/typed_scalar.h"

namespace Mayo {

// Settings GroupIndex
struct Settings_GroupTag {};
using Settings_GroupIndex = TypedScalar<int, Settings_GroupTag>;


// Settings SectionIndex
struct Settings_SectionTag {};
class Settings_SectionIndex : public TypedScalar<int, Settings_SectionTag> {
public:
    Settings_SectionIndex() = default;
    explicit Settings_SectionIndex(Settings_GroupIndex group, ScalarType section)
        : TypedScalar<ScalarType, Settings_SectionTag>(section),
          m_group(group)
    {}

    Settings_GroupIndex group() const { return m_group; }

private:
    Settings_GroupIndex m_group;
};


// Settings SettingIndex
struct Settings_SettingTag {};
class Settings_SettingIndex : public TypedScalar<int, Settings_SettingTag> {
public:
    Settings_SettingIndex() = default;
    explicit Settings_SettingIndex(Settings_SectionIndex section, ScalarType setting)
        : TypedScalar<ScalarType, Settings_SettingTag>(setting),
          m_section(section)
    {}

    Settings_GroupIndex group() const { return m_section.group(); }
    Settings_SectionIndex section() const { return m_section; }

private:
    Settings_SectionIndex m_section;
};

} // namespace Mayo
