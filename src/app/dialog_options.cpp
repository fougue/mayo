/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_options.h"

#include "../base/cpp_utils.h"
#include "../base/settings.h"
#include "../qtbackend/qsettings_storage.h"
#include "../qtcommon/qstring_conv.h"
#include "app_module.h"
#include "item_view_buttons.h"
#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "theme.h"
#include "ui_dialog_options.h"

#include <QtCore/QSettings>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QPushButton>

namespace Mayo {

namespace {

// Encodes the index of an item node in the Settings tree(group+section)
// The 4 "high" bytes are used for the group id
// The 4 "low" bytes are used for the section id
using SettingNodeId = uint32_t;

SettingNodeId toSettingNodeId(Settings::GroupIndex indexGroup)
{
    return uint16_t(indexGroup.get()) << 16;
}

SettingNodeId toSettingNodeId(Settings::SectionIndex indexSection)
{
    const auto groupId = uint16_t(indexSection.group().get());
    const auto sectionId = uint16_t(indexSection.get());
    return (groupId << 16) | sectionId;
}

Settings::GroupIndex toGroupIndex(SettingNodeId nodeId)
{
    const int index = (nodeId & 0xFFFF0000) >> 16;
    return Settings::GroupIndex(index);
}

Settings::SectionIndex toSectionIndex(SettingNodeId nodeId)
{
    const int index = (nodeId & 0x0000FFFF);
    return Settings::SectionIndex(toGroupIndex(nodeId), index);
}

// Specific role for storing the SettingNodeId value of an item node in the Settings tree
constexpr int ItemSettingNodeId_Role = Qt::UserRole + 1;

// Create the tree model corresponding to the Settings tree(groups and sections)
QAbstractItemModel* createGroupSectionModel(const Settings* settings, QObject* parent = nullptr)
{
    auto model = new QStandardItemModel(parent);
    QStandardItem* itemRoot = model->invisibleRootItem();
    for (int iGroup = 0; iGroup < settings->groupCount(); ++iGroup) {
        const Settings::GroupIndex indexGroup(iGroup);
        const QString titleGroup = to_QString(settings->groupTitle(indexGroup));
        auto itemGroup = new QStandardItem(titleGroup);
        itemGroup->setData(toSettingNodeId(indexGroup), ItemSettingNodeId_Role);
        for (int iSection = 0; iSection < settings->sectionCount(indexGroup); ++iSection) {
            const Settings::SectionIndex indexSection(indexGroup, iSection);
            if (settings->isDefaultGroupSection(indexSection))
                continue;

            if (settings->settingCount(indexSection) == 0)
                continue;

            const QString titleSection = to_QString(settings->sectionTitle(indexSection));
            auto itemSection = new QStandardItem(titleSection);
            itemSection->setData(toSettingNodeId(indexSection), ItemSettingNodeId_Role);
            itemGroup->appendRow(itemSection);
        }

        itemRoot->appendRow(itemGroup);
    }

    return model;
}

// Provides a custom style for deactivating mouse wheel interaction with combo boxes
class CustomStyle : public QProxyStyle {
public:
    CustomStyle()
        : QProxyStyle(qApp->style())
    {}

    static QStyle* instance() {
        static CustomStyle style;
        return &style;
    }

    int styleHint(
            StyleHint hint,
            const QStyleOption* option,
            const QWidget* widget,
            QStyleHintReturn* returnData
        ) const override
    {
        if (hint == QStyle::SH_ComboBox_AllowWheelScrolling)
            return 0;
        else
            return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

// Reserved name for Property(ie setting) editors. QObject::objectName() will return this name
static const char reservedPropertyEditorName[] = "__Mayo_propertyEditor";

} // namespace

DialogOptions::DialogOptions(Settings* settings, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogOptions),
      m_editorFactory(new DefaultPropertyEditorFactory),
      m_settings(settings)
{
    m_ui->setupUi(this);
    m_ui->listWidget_Settings->setStyle(CustomStyle::instance());

    auto treeModel = createGroupSectionModel(settings, this);
    m_ui->treeView_GroupSections->setModel(treeModel);
    m_ui->treeView_GroupSections->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->treeView_GroupSections->expandAll();

    // Add action "Restore defaults" for group/section items in "left-side" tree view
    {
        auto treeViewBtns = new ItemViewButtons(m_ui->treeView_GroupSections, this);
        constexpr int idBtnRestore = 1;
        treeViewBtns->addButton(
                    idBtnRestore,
                    mayoTheme()->icon(Theme::Icon::Reload),
                    tr("Restore default values")
        );
        treeViewBtns->setButtonDetection(idBtnRestore, -1, {});
        treeViewBtns->setButtonDisplayColumn(idBtnRestore, 0);
        treeViewBtns->setButtonDisplayModes(idBtnRestore, ItemViewButtons::DisplayWhenItemSelected);
        treeViewBtns->setButtonItemSide(idBtnRestore, ItemViewButtons::ItemRightSide);
        const int iconSize = this->style()->pixelMetric(QStyle::PM_ListViewIconSize);
        treeViewBtns->setButtonIconSize(idBtnRestore, QSize(iconSize * 0.66, iconSize * 0.66));
        treeViewBtns->installDefaultItemDelegate();
        QObject::connect(
                    treeViewBtns, &ItemViewButtons::buttonClicked,
                    this, [=](int btnId, const QModelIndex& index) {
            if (btnId == idBtnRestore && index.isValid())
                this->handleTreeViewButtonClick_restoreDefaults(index);
        });
    }

    // Build "right-side" model(setting items)
    const QFont fontItemGroupSection =
            QtGuiUtils::FontChange(m_ui->listWidget_Settings->font())
            .bold(true).scalePointSizeF(1.7);

    for (int iGroup = 0; iGroup < settings->groupCount(); ++iGroup) {
        const Settings::GroupIndex indexGroup(iGroup);
        const QString titleGroup = to_QString(settings->groupTitle(indexGroup));
        auto listItemGroup = new QListWidgetItem(titleGroup, m_ui->listWidget_Settings);
        listItemGroup->setFont(fontItemGroupSection);
        listItemGroup->setData(ItemSettingNodeId_Role, toSettingNodeId(indexGroup));
        for (int iSection = 0; iSection < settings->sectionCount(indexGroup); ++iSection) {
            const Settings::SectionIndex indexSection(indexGroup, iSection);
            const int settingCount = settings->settingCount(indexSection);
            if (settingCount == 0)
                continue; // Skip empty section

            if (!settings->isDefaultGroupSection(indexSection)) {
                const QString titleSection = tr("%1 / %2").arg(titleGroup, to_QString(settings->sectionTitle(indexSection)));
                auto listItemSection = new QListWidgetItem(titleSection, m_ui->listWidget_Settings);
                listItemSection->setFont(fontItemGroupSection);
                listItemSection->setData(ItemSettingNodeId_Role, toSettingNodeId(indexSection));
            }

            for (int iSetting = 0; iSetting < settingCount; ++iSetting) {
                Property* property = settings->property(Settings::SettingIndex(indexSection, iSetting));
                if (!property->isUserVisible())
                    continue;

                auto widget = this->createEditor(property, nullptr);
                auto listItemSetting = new QListWidgetItem;
                listItemSetting->setData(ItemSettingNodeId_Role, toSettingNodeId(indexSection));
                listItemSetting->setSizeHint(widget->sizeHint());
                m_ui->listWidget_Settings->addItem(listItemSetting);
                m_ui->listWidget_Settings->setItemWidget(listItemSetting, widget);
                m_mapSettingEditor.insert({ property, widget });
            } // endfor(setting)
        } // endfor(section)
    }

    // Enable/disable editor widget when the corresponding setting status is changed
    m_connSettingsEnabled = settings->signalEnabled.connectSlot([=](const Property* setting, bool on) {
        QWidget* editor = CppUtils::findValue(setting, m_mapSettingEditor);
        if (editor)
            editor->setEnabled(on);
    });

    // Backup initial value of changed settings, so they can be restored on dialog cancellation
    m_connSettingsAboutToChange = settings->signalAboutToChange.connectSlot([=](Property* property) {
        if (m_mapSettingInitialValue.find(property) == m_mapSettingInitialValue.cend()) {
            const Settings::Variant propertyValue = settings->propertyValueConversion().toVariant(*property);
            m_mapSettingInitialValue.insert({ property, propertyValue});
        }
    });

    // Synchronize editor widget when value of the corresponding property is changed
    m_connSettingsChanged = settings->signalChanged.connectSlot([=](const Property* property) {
        auto itFound = m_mapSettingEditor.find(property);
        if (itFound != m_mapSettingEditor.cend())
            this->syncEditor(itFound->second);
    });

    // When a setting is clicked in the "right-side" view then scroll to and select corresponding
    // tree item in the "left-side" view
    QObject::connect(m_ui->listWidget_Settings, &QListWidget::currentRowChanged, this, [=](int listRow) {
        auto listItem = m_ui->listWidget_Settings->item(listRow);
        const QVariant variantNodeId = listItem ? listItem->data(ItemSettingNodeId_Role) : QVariant();
        const Qt::MatchFlags matchFlags = Qt::MatchRecursive | Qt::MatchExactly;
        const QModelIndex indexFirst = treeModel->index(0, 0);
        const QModelIndexList indexList = treeModel->match(indexFirst, ItemSettingNodeId_Role, variantNodeId, 1, matchFlags);
        if (!indexList.isEmpty()) {
            m_ui->treeView_GroupSections->scrollTo(indexList.front(), QAbstractItemView::PositionAtTop);
            QSignalBlocker _(m_ui->treeView_GroupSections);
            m_ui->treeView_GroupSections->selectionModel()->select(
                        indexList.front(), QItemSelectionModel::SelectCurrent
            );
        }
    });

    // When a group/section is clicked in the "left-side" view then scroll to the list item in
    // the "right-side" view
    QObject::connect(
                m_ui->treeView_GroupSections->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [=](const QModelIndex& current) {
        const QAbstractItemModel* settingsModel = m_ui->listWidget_Settings->model();
        const QVariant variantNodeId = current.data(ItemSettingNodeId_Role);
        const QModelIndex indexFirst = settingsModel->index(0, 0);
        const QModelIndexList indexList = settingsModel->match(
                    indexFirst, ItemSettingNodeId_Role, variantNodeId, 1, Qt::MatchExactly
        );
        if (!indexList.isEmpty())
            m_ui->listWidget_Settings->scrollTo(indexList.front(), QAbstractItemView::PositionAtTop);
    });

    // Action for "Cancel" button : restore changed properties to their initial values
    QObject::connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogOptions::cancelChanges);

    // Action for "Restore defaults" button
    auto btnRestoreDefaults = m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    QObject::connect(btnRestoreDefaults, &QPushButton::clicked, this, [=]{ settings->resetAll(); });

    // Actions for "Exchange" button
    auto btnExchange = m_ui->buttonBox->addButton(tr("Exchange"), QDialogButtonBox::ActionRole);
    {
        auto menu = new QMenu(btnExchange);
        menu->addAction(tr("Load from file..."), this, &DialogOptions::loadFromFile);
        menu->addAction(tr("Save as..."), this, &DialogOptions::saveAs);
        btnExchange->setMenu(menu);
    }
}

DialogOptions::~DialogOptions()
{
    m_connSettingsAboutToChange.disconnect();
    m_connSettingsChanged.disconnect();
    m_connSettingsEnabled.disconnect();
    delete m_ui;
}

void DialogOptions::setPropertyEditorFactory(std::unique_ptr<IPropertyEditorFactory> editorFactory)
{
    m_editorFactory = std::move(editorFactory);
}

QWidget* DialogOptions::createEditor(Property* property, QWidget* parentWidget) const
{
    if (!property)
        return nullptr;

    auto widget = new QWidget(parentWidget);
    auto labelName = new QLabel(QString("<b>%1</b>").arg(to_QString(property->label())), widget);
    auto widgetLayout = new QVBoxLayout(widget);
    widgetLayout->addWidget(labelName);

    auto panelEditor = new QWidget(widget);
    widgetLayout->addWidget(panelEditor);
    {
        auto panelEditorLayout = new QVBoxLayout(panelEditor);
        panelEditorLayout->setContentsMargins(0, 0, 0, 10);
        if (!property->description().empty()) {
            auto labelDescription = new QLabel(to_QString(property->description()), panelEditor);
            labelDescription->setMinimumWidth(600);
            labelDescription->setWordWrap(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            labelDescription->setTextFormat(Qt::MarkdownText);
#else
            // TODO Convert property description from markdown to HTML
            labelDescription->setTextFormat(Qt::RichText);
#endif
            labelDescription->setAlignment(Qt::AlignLeft);
            panelEditorLayout->addWidget(labelDescription);
            //panelLayout->itemAt(1)->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        }

        auto editor = m_editorFactory->createEditor(property, panelEditor);
        if (editor) {
            editor->setObjectName(reservedPropertyEditorName);
            panelEditorLayout->addWidget(editor);
            for (QWidget* childWidget : panelEditor->findChildren<QWidget*>()) {
                if (qobject_cast<QComboBox*>(childWidget) || qobject_cast<QLineEdit*>(childWidget))
                    childWidget->setMaximumWidth(250);

                if (qobject_cast<QAbstractSpinBox*>(childWidget))
                    childWidget->setMaximumWidth(150);
            }

            panelEditorLayout->addWidget(editor);
        }
    }

    widgetLayout->setSizeConstraint(QLayout::SetMaximumSize);
    widget->setEnabled(property->isEnabled());
    return widget;
}

void DialogOptions::syncEditor(QWidget* editor)
{
    if (editor && editor->objectName() != reservedPropertyEditorName)
        editor = editor->findChild<QWidget*>(reservedPropertyEditorName);

    m_editorFactory->syncEditorWithProperty(editor);
}

void DialogOptions::loadFromFile()
{
    const QString startDirPath = QString();
    const QString filepath = QFileDialog::getOpenFileName(
                this, tr("Choose INI file"), startDirPath, tr("INI files(*.ini)"));
    if (filepath.isEmpty())
        return;

    const QFileInfo fi(filepath);
    if (!fi.exists()) {
        QtWidgetsUtils::asyncMsgBoxCritical(this, tr("Error"), tr("'%1' doesn't exist").arg(filepath));
        return;
    }

    if (!fi.isReadable()) {
        QtWidgetsUtils::asyncMsgBoxCritical(this, tr("Error"), tr("'%1' is not readable").arg(filepath));
        return;
    }

    QSettingsStorage fileSettings(filepath, QSettings::IniFormat);
    m_settings->loadFrom(fileSettings, &AppModule::excludeSettingPredicate);
}

void DialogOptions::saveAs()
{
    const QString startDirPath = QString();
    const QString filepath = QFileDialog::getSaveFileName(
                this, tr("Choose INI file"), startDirPath, tr("INI files(*.ini)")
    );
    if (filepath.isEmpty())
        return;

    QSettingsStorage fileSettings(filepath, QSettings::IniFormat);
    m_settings->saveAs(&fileSettings, &AppModule::excludeSettingPredicate);
    fileSettings.sync();
    if (fileSettings.get().status() != QSettings::NoError)
        QtWidgetsUtils::asyncMsgBoxCritical(this, tr("Error"), tr("Error when writing to '%1'").arg(filepath));
}

void DialogOptions::cancelChanges()
{
    m_connSettingsAboutToChange.block(true);
    m_connSettingsChanged.block(true);
    for (const auto& [prop, propInitialValue] : m_mapSettingInitialValue)
        m_settings->propertyValueConversion().fromVariant(prop, propInitialValue);

    m_connSettingsAboutToChange.block(false);
    m_connSettingsChanged.block(false);
}

void DialogOptions::handleTreeViewButtonClick_restoreDefaults(const QModelIndex& index)
{
    const SettingNodeId nodeId = index.data(ItemSettingNodeId_Role).toUInt();
    const bool isGroup = !index.parent().isValid();
    if (isGroup) {
        const Settings::GroupIndex groupIndex = toGroupIndex(nodeId);
        const Settings::SectionIndex firstSectionIndex(groupIndex, 0);
        const bool defaultSectionNotEmpty =
                m_settings->isDefaultGroupSection(firstSectionIndex)
                && m_settings->settingCount(firstSectionIndex) > 0;
        if (defaultSectionNotEmpty && m_settings->sectionCount(groupIndex) > 1) {
            auto menu = new QMenu(this);
            menu->addAction(tr("Restore values for default section only"), this, [=]{
                m_settings->resetSection(firstSectionIndex);
            });
            menu->addAction(tr("Restore values for the whole group"), this, [=]{
                m_settings->resetGroup(groupIndex);
            });
            QtWidgetsUtils::asyncMenuExec(menu);
        }
        else {
            m_settings->resetGroup(groupIndex);
        }
    }
    else {
        m_settings->resetSection(toSectionIndex(nodeId));
    }
}

} // namespace Mayo
