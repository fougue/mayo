/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
#include "../base/settings.h"
#include "../base/text_id.h"
#include "commands_api.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Mayo {

class Settings;

// Exposes the keyboard shortcuts of the application commands as user settings
//
// One setting is registered per Command object, the setting identifier being the command name
// (eg "open-docs") so that persisted values do not depend on labels or translations.
// Shortcuts are stored as portable text(eg "Ctrl+Shift+O"), an empty value meaning "no shortcut".
// Any change is applied immediately to the QAction of the corresponding Command.
class CommandShortcuts : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::CommandShortcuts)
public:
    // Description of a configurable command
    struct Entry {
        std::string name; // Command identifier
        std::string label; // Command name, as displayed to the user
        std::string defaultText; // Default shortcut, as portable text
        QList<QKeySequence> defaultShortcuts; // Default shortcuts(see QAction::shortcuts())
        PropertyKeySequence* property = nullptr; // Setting holding the user shortcut
        QAction* action = nullptr; // Action of the command
    };

    // Registers the shortcuts of all the commands of `cmdContainer`
    // The commands must have created their QAction beforehand
    CommandShortcuts(Settings* settings, const CommandContainer* cmdContainer);
    ~CommandShortcuts() override;

    // Table of the application shortcuts, available once the main window has been created
    static CommandShortcuts* instance() { return s_instance; }

    const std::vector<Entry>& entries() const { return m_vecEntry; }

    // Assigns the shortcut `portableText`(empty to remove any shortcut) to command `name`
    // Returns false if `name` is not a command of the table
    bool setShortcut(std::string_view name, std::string_view portableText);

    // Restores the default shortcut of command `name`
    // Returns false if `name` is not a command of the table
    bool restoreDefaultShortcut(std::string_view name);

    // Labels of the commands, other than `name`, already assigned to `portableText`
    std::vector<std::string> findConflicts(std::string_view name, std::string_view portableText) const;

    // Command identifier of the command owning `property`, empty if `property` is not a shortcut
    std::string_view commandName(const Property* property) const;

    void restoreDefaults() override;

    // Settings group holding the shortcut settings
    const Settings::GroupIndex groupId_shortcuts;

private:
    void applyShortcut(const Entry& entry);

    Settings::SectionIndex m_sectionId_commands;
    std::vector<Entry> m_vecEntry;
    std::vector<std::unique_ptr<PropertyKeySequence>> m_vecProperty;
    SignalConnectionHandle m_connPropertyChanged;
    static CommandShortcuts* s_instance;
};

} // namespace Mayo
