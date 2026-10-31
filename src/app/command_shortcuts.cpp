/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "command_shortcuts.h"

#include "../qtcommon/qstring_conv.h"

#include <QtGui/QKeySequence>

namespace Mayo {

CommandShortcuts* CommandShortcuts::s_instance = nullptr;

namespace {

// Returns the portable text(eg "Ctrl+O") of the primary shortcut of 'action'
std::string primaryShortcutText(const QAction* action)
{
    const QList<QKeySequence> shortcuts = action ? action->shortcuts() : QList<QKeySequence>{};
    return shortcuts.empty()
        ? std::string()
        : to_stdString(shortcuts.front().toString(QKeySequence::PortableText));
}

} // namespace

CommandShortcuts::CommandShortcuts(Settings* settings, const CommandContainer* cmdContainer)
    : PropertyGroup(settings),
      groupId_shortcuts(settings->addGroup(textId("Shortcuts"))),
      m_sectionId_commands(settings->addSection(groupId_shortcuts, textId("Commands")))
{
    if (cmdContainer) {
        cmdContainer->foreachCommand([&](std::string_view name, Command* cmd) {
            QAction* action = cmd ? cmd->action() : nullptr;
            if (!action)
                return; // Command without UI binding : nothing to configure

            Entry entry;
            entry.name = name;
            entry.label = to_stdString(QString(action->text()).remove(QLatin1Char('&')));
            entry.defaultShortcuts = action->shortcuts();
            entry.defaultText = primaryShortcutText(action);
            entry.action = action;

            auto property = std::make_unique<PropertyKeySequence>(this, textId(name));
            property->setValue(entry.defaultText);
            entry.property = property.get();
            settings->addSetting(entry.property, m_sectionId_commands);

            m_vecProperty.push_back(std::move(property));
            m_vecEntry.push_back(std::move(entry));
        });
    }

    settings->addResetFunction(m_sectionId_commands, [this]{ this->restoreDefaults(); });
    m_connPropertyChanged = this->signalPropertyChanged.connectSlot([this](Property* property) {
        for (const Entry& entry : m_vecEntry) {
            if (entry.property == property) {
                this->applyShortcut(entry);
                break;
            }
        }
    });

    s_instance = this;
}

CommandShortcuts::~CommandShortcuts()
{
    if (s_instance == this)
        s_instance = nullptr;
}

void CommandShortcuts::applyShortcut(const Entry& entry)
{
    if (!entry.action || !entry.property)
        return;

    const std::string text = entry.property->value();
    if (text.empty()) {
        entry.action->setShortcuts({});
    }
    else if (text == entry.defaultText) {
        // Keep the shortcuts defined by the command itself : a standard key can be bound to
        // several key sequences(eg both "Ctrl+=" and "Ctrl++" for QKeySequence::ZoomIn)
        entry.action->setShortcuts(entry.defaultShortcuts);
    }
    else {
        entry.action->setShortcuts({
            QKeySequence::fromString(to_QString(text), QKeySequence::PortableText)
        });
    }
}

bool CommandShortcuts::setShortcut(std::string_view name, std::string_view portableText)
{
    for (Entry& entry : m_vecEntry) {
        if (entry.name == name) {
            entry.property->setValue(std::string(portableText));
            return true;
        }
    }

    return false;
}

bool CommandShortcuts::restoreDefaultShortcut(std::string_view name)
{
    for (Entry& entry : m_vecEntry) {
        if (entry.name == name) {
            entry.property->setValue(entry.defaultText);
            return true;
        }
    }

    return false;
}

std::vector<std::string> CommandShortcuts::findConflicts(
        std::string_view name, std::string_view portableText
    ) const
{
    std::vector<std::string> vecConflictLabel;
    const QKeySequence seq = QKeySequence::fromString(
        to_QString(portableText), QKeySequence::PortableText
    );
    if (seq.isEmpty())
        return vecConflictLabel;

    for (const Entry& entry : m_vecEntry) {
        if (entry.name == name)
            continue;

        const std::string text = entry.property->value();
        if (text.empty())
            continue;

        const QKeySequence seqOther = QKeySequence::fromString(
            to_QString(text), QKeySequence::PortableText
        );
        if (seqOther == seq)
            vecConflictLabel.push_back(entry.label);
    }

    return vecConflictLabel;
}

std::string_view CommandShortcuts::commandName(const Property* property) const
{
    for (const Entry& entry : m_vecEntry) {
        if (entry.property == property)
            return entry.name;
    }

    return {};
}

void CommandShortcuts::restoreDefaults()
{
    for (Entry& entry : m_vecEntry)
        entry.property->setValue(entry.defaultText);
}

} // namespace Mayo
