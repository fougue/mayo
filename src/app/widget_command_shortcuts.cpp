/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "widget_command_shortcuts.h"

#include "../qtcommon/qstring_conv.h"

#include <QtCore/QHash>
#include <QtCore/QRegularExpression>
#include <QtGui/QKeySequence>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QKeySequenceEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

#include <algorithm>
#include <unordered_map>

namespace Mayo {

namespace {

constexpr int Column_Command = 0;
constexpr int Column_Shortcut = 1;
constexpr int Column_Restore = 2;

// Category of the commands that are not reachable from a menu(eg commands of the 3D view)
constexpr int CategoryOrder_Misc = 1000;

// Returns 'text' without the Qt mnemonic characters(eg "&File" -> "File", "Files(&F)" -> "Files")
QString plainText(QString text)
{
    static const QRegularExpression mnemonicSuffix(QStringLiteral("\\(&.\\)$"));
    return text.remove(mnemonicSuffix).remove(QLatin1Char('&'));
}

// Records in 'mapActionCategory' the menu path(eg "Display / Visibility") of the actions of 'menu'
void recordMenuCategories(
        const QMenu* menu,
        const QString& parentPath,
        QHash<const QAction*, QString>& mapActionCategory,
        QHash<QString, int>& mapCategoryOrder
    )
{
    if (!menu)
        return;

    const QString path = parentPath.isEmpty()
        ? plainText(menu->title())
        : parentPath + QStringLiteral(" / ") + plainText(menu->title());
    if (!mapCategoryOrder.contains(path))
        mapCategoryOrder.insert(path, int(mapCategoryOrder.size()));

    for (const QAction* action : menu->actions()) {
        if (!action || action->isSeparator())
            continue;

        if (const QMenu* subMenu = action->menu())
            recordMenuCategories(subMenu, path, mapActionCategory, mapCategoryOrder);
        else
            mapActionCategory.insert(action, path);
    }
}

// Returns the menu bar of the application main window, if any
const QMenuBar* mainWindowMenuBar()
{
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        auto mainWindow = qobject_cast<QMainWindow*>(widget);
        if (mainWindow && mainWindow->menuBar())
            return mainWindow->menuBar();
    }

    return nullptr;
}

// Returns the text of the shortcut editor, for a property value as portable text
QKeySequence keySequenceFromText(std::string_view portableText)
{
    return QKeySequence::fromString(to_QString(portableText), QKeySequence::PortableText);
}

} // namespace

WidgetCommandShortcuts::WidgetCommandShortcuts(CommandShortcuts* shortcuts, QWidget* parent)
    : QWidget(parent),
      m_shortcuts(shortcuts)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    m_lineEditFilter = new QLineEdit(this);
    m_lineEditFilter->setClearButtonEnabled(true);
    m_lineEditFilter->setPlaceholderText(tr("Filter commands by name or shortcut ..."));
    layout->addWidget(m_lineEditFilter);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setColumnCount(3);
    m_treeWidget->setHeaderLabels({ tr("Command"), tr("Shortcut"), QString{} });
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeWidget->header()->setSectionResizeMode(Column_Command, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(Column_Shortcut, QHeaderView::Fixed);
    m_treeWidget->header()->setSectionResizeMode(Column_Restore, QHeaderView::Fixed);
    m_treeWidget->header()->resizeSection(Column_Shortcut, 170);
    m_treeWidget->header()->resizeSection(Column_Restore, 26);
    m_treeWidget->setMinimumHeight(360);
    layout->addWidget(m_treeWidget, 1);

    m_labelStatus = new QLabel(this);
    m_labelStatus->setWordWrap(true);
    layout->addWidget(m_labelStatus);

    QObject::connect(m_lineEditFilter, &QLineEdit::textChanged, this, &WidgetCommandShortcuts::applyFilter);
    if (m_shortcuts) {
        m_shortcuts->signalPropertyChanged.connectSlot([this](Property*) {
            this->syncEditors();
        });
    }

    this->populateTable();
    this->syncEditors();

    // A key stroke must not be recorded by the first shortcut editor of the table
    m_lineEditFilter->setFocus();
}

WidgetCommandShortcuts::~WidgetCommandShortcuts()
{
    // The editors are destroyed by the QWidget destructor, after this one : the focus out events
    // they emit during their destruction must not re-enter the synchronization code.
    m_isSynchronizing = true;
    for (Row& row : m_vecRow) {
        if (row.editor)
            row.editor->disconnect(this);
    }
}

void WidgetCommandShortcuts::populateTable()
{
    if (!m_shortcuts)
        return;

    // Record the category(ie menu path) of every command action
    QHash<const QAction*, QString> mapActionCategory;
    QHash<QString, int> mapCategoryOrder;
    const QMenuBar* menuBar = mainWindowMenuBar();
    for (const QAction* menuAction : menuBar ? menuBar->actions() : QList<QAction*>{}) {
        const QMenu* menu = menuAction ? menuAction->menu() : nullptr;
        if (menu)
            recordMenuCategories(menu, QString{}, mapActionCategory, mapCategoryOrder);
    }

    const QString strCategoryMisc = tr("Other commands");

    // Sort the commands by category, then by name : the category items are then created in the same
    // order as the menus, without any re-ordering of the tree afterwards
    struct SortedEntry {
        const CommandShortcuts::Entry* entry = nullptr;
        QString category;
        int categoryOrder = CategoryOrder_Misc;
    };
    std::vector<SortedEntry> vecSortedEntry;
    for (const CommandShortcuts::Entry& entry : m_shortcuts->entries()) {
        SortedEntry sortedEntry;
        sortedEntry.entry = &entry;
        auto itCategory = mapActionCategory.find(entry.action);
        if (itCategory != mapActionCategory.end())
            sortedEntry.category = itCategory.value();
        else
            sortedEntry.category = strCategoryMisc;
        sortedEntry.categoryOrder = mapCategoryOrder.value(sortedEntry.category, CategoryOrder_Misc);
        vecSortedEntry.push_back(sortedEntry);
    }
    std::sort(vecSortedEntry.begin(), vecSortedEntry.end(),
        [](const SortedEntry& lhs, const SortedEntry& rhs) {
            if (lhs.categoryOrder != rhs.categoryOrder)
                return lhs.categoryOrder < rhs.categoryOrder;
            return QString::localeAwareCompare(
                to_QString(lhs.entry->label), to_QString(rhs.entry->label)
            ) < 0;
        }
    );

    std::unordered_map<QString, QTreeWidgetItem*> mapCategoryItem;
    for (const SortedEntry& sortedEntry : vecSortedEntry) {
        const CommandShortcuts::Entry& entry = *sortedEntry.entry;

        QTreeWidgetItem*& itemCategory = mapCategoryItem[sortedEntry.category];
        if (!itemCategory) {
            itemCategory = new QTreeWidgetItem(m_treeWidget);
            itemCategory->setText(Column_Command, sortedEntry.category);
            itemCategory->setFirstColumnSpanned(true);
            itemCategory->setFlags(Qt::ItemIsEnabled);
            QFont fontCategory = itemCategory->font(Column_Command);
            fontCategory.setBold(true);
            itemCategory->setFont(Column_Command, fontCategory);
            itemCategory->setExpanded(true);
        }

        auto item = new QTreeWidgetItem(itemCategory);
        item->setText(Column_Command, to_QString(entry.label));
        item->setToolTip(Column_Command, tr("Command identifier: %1").arg(to_QString(entry.name)));
        item->setFlags(Qt::ItemIsEnabled);

        auto editor = new QKeySequenceEdit(m_treeWidget);
        editor->setToolTip(tr("Press the key combination to assign it, press Backspace to remove it"));
        if (auto lineEdit = editor->findChild<QLineEdit*>())
            lineEdit->setPlaceholderText(tr("None"));
        m_treeWidget->setItemWidget(item, Column_Shortcut, editor);

        auto btnRestore = new QPushButton(QStringLiteral("\u21ba"), m_treeWidget);
        btnRestore->setFlat(true);
        btnRestore->setFixedWidth(22);
        btnRestore->setToolTip(tr("Restore default shortcut (%1)").arg(
            entry.defaultText.empty() ? tr("None") : to_QString(entry.defaultText)
        ));
        m_treeWidget->setItemWidget(item, Column_Restore, btnRestore);

        Row row;
        row.entry = &entry;
        row.item = item;
        row.editor = editor;
        m_vecRow.push_back(row);

        QObject::connect(editor, &QKeySequenceEdit::editingFinished, this, [=]{
            if (m_isSynchronizing || !m_shortcuts)
                return;

            const QKeySequence seq = editor->keySequence();
            m_shortcuts->setShortcut(
                row.entry->name, to_stdString(seq.toString(QKeySequence::PortableText))
            );
        });
        QObject::connect(btnRestore, &QPushButton::clicked, this, [=]{
            if (m_shortcuts)
                m_shortcuts->restoreDefaultShortcut(row.entry->name);
        });
    }
}

void WidgetCommandShortcuts::syncEditors()
{
    if (!m_shortcuts || m_isSynchronizing)
        return;

    m_isSynchronizing = true;
    for (Row& row : m_vecRow) {
        if (!row.editor)
            continue;

        const QKeySequence seq = keySequenceFromText(row.entry->property->value());
        if (row.editor->keySequence() != seq)
            row.editor->setKeySequence(seq);
    }
    m_isSynchronizing = false;

    this->updateStatus();
}

void WidgetCommandShortcuts::updateStatus()
{
    if (!m_shortcuts || !m_labelStatus)
        return;

    int shortcutCount = 0;
    int conflictCount = 0;
    for (Row& row : m_vecRow) {
        if (!row.editor)
            continue;

        const std::string text = row.entry->property->value();
        if (!text.empty())
            ++shortcutCount;

        const std::vector<std::string> vecConflictLabel = m_shortcuts->findConflicts(
            row.entry->name, text
        );
        if (vecConflictLabel.empty()) {
            row.editor->setStyleSheet({});
            row.editor->setToolTip(
                tr("Press the key combination to assign it, press Backspace to remove it")
            );
        }
        else {
            QStringList listLabel;
            for (const std::string& label : vecConflictLabel)
                listLabel.push_back(to_QString(label));
            row.editor->setStyleSheet(QStringLiteral("QKeySequenceEdit { border: 1px solid #d9534f; }"));
            row.editor->setToolTip(tr("Also used by: %1").arg(listLabel.join(QStringLiteral(", "))));
            ++conflictCount;
        }
    }

    if (conflictCount == 0) {
        m_labelStatus->setStyleSheet({});
        m_labelStatus->setText(tr("%1 commands, %2 with a shortcut")
                               .arg(int(m_vecRow.size())).arg(shortcutCount));
    }
    else {
        m_labelStatus->setStyleSheet(QStringLiteral("color: #d9534f;"));
        m_labelStatus->setText(tr("Warning: %1 shortcuts are used by several commands").arg(conflictCount));
    }
}

void WidgetCommandShortcuts::applyFilter(const QString& text)
{
    const QString strSearch = text.trimmed();
    for (Row& row : m_vecRow) {
        if (!row.item || !row.editor)
            continue;

        bool isMatch = strSearch.isEmpty();
        if (!isMatch) {
            const QString strShortcut = row.editor->keySequence().toString(QKeySequence::NativeText);
            isMatch = row.item->text(Column_Command).contains(strSearch, Qt::CaseInsensitive)
                      || to_QString(row.entry->name).contains(strSearch, Qt::CaseInsensitive)
                      || strShortcut.contains(strSearch, Qt::CaseInsensitive);
        }
        row.item->setHidden(!isMatch);
    }

    // Hide the categories that do not contain any matching command
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* itemCategory = m_treeWidget->topLevelItem(i);
        bool hasVisibleChild = false;
        for (int iChild = 0; iChild < itemCategory->childCount() && !hasVisibleChild; ++iChild)
            hasVisibleChild = !itemCategory->child(iChild)->isHidden();
        itemCategory->setHidden(!hasVisibleChild);
        if (hasVisibleChild)
            itemCategory->setExpanded(true);
    }
}

} // namespace Mayo
