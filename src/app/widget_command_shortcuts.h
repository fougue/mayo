/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "command_shortcuts.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

#include <vector>

class QLabel;
class QKeySequenceEdit;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

namespace Mayo {

// Widget to edit the keyboard shortcuts of the application commands
//
// The commands are listed by category(ie the menu they belong to) and the list can be filtered, so
// that a given command is quickly found. A shortcut already used by another command is highlighted.
class WidgetCommandShortcuts : public QWidget {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::WidgetCommandShortcuts)
public:
    explicit WidgetCommandShortcuts(CommandShortcuts* shortcuts, QWidget* parent = nullptr);
    ~WidgetCommandShortcuts() override;

private:
    struct Row {
        const CommandShortcuts::Entry* entry = nullptr;
        QTreeWidgetItem* item = nullptr; // Item owned by the tree widget
        QPointer<QKeySequenceEdit> editor; // Editor owned by the tree widget
    };

    void populateTable();
    void syncEditors();
    void updateStatus();
    void applyFilter(const QString& text);

    CommandShortcuts* m_shortcuts = nullptr;
    QLineEdit* m_lineEditFilter = nullptr;
    QTreeWidget* m_treeWidget = nullptr;
    QLabel* m_labelStatus = nullptr;
    std::vector<Row> m_vecRow;
    bool m_isSynchronizing = false;
};

} // namespace Mayo
