/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_options.h"

#include "../base/settings.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "ui_dialog_options.h"

#include <QtGui/QStandardItemModel>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QPushButton>

namespace Mayo {

namespace {

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

enum { ItemSettingNodeId_Role = Qt::UserRole + 1 };

QAbstractItemModel* createGroupSectionModel(const Settings* settings, QObject* parent = nullptr)
{
    auto model = new QStandardItemModel(parent);
    QStandardItem* itemRoot = model->invisibleRootItem();
    for (int iGroup = 0; iGroup < settings->groupCount(); ++iGroup) {
        const Settings::GroupIndex indexGroup(iGroup);
        const QString titleGroup = settings->groupTitle(indexGroup);
        auto itemGroup = new QStandardItem(titleGroup);
        itemGroup->setData(toSettingNodeId(indexGroup), ItemSettingNodeId_Role);
        for (int iSection = 0; iSection < settings->sectionCount(indexGroup); ++iSection) {
            const Settings::SectionIndex indexSection(indexGroup, iSection);
            if (settings->isDefaultGroupSection(indexSection))
                continue;

            if (settings->settingCount(indexSection) == 0)
                continue;

            const QString titleSection = settings->sectionTitle(indexSection);
            auto itemSection = new QStandardItem(titleSection);
            itemSection->setData(toSettingNodeId(indexSection), ItemSettingNodeId_Role);
            itemGroup->appendRow(itemSection);
        }

        itemRoot->appendRow(itemGroup);
    }

    return model;
}

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
            QStyleHintReturn* returnData) const override
    {
        if (hint == QStyle::SH_ComboBox_AllowWheelScrolling)
            return 0;
        else
            return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

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

    QFont fontItemGroupSection = m_ui->listWidget_Settings->font();
    fontItemGroupSection.setBold(true);
    fontItemGroupSection.setPointSizeF(1.7 * fontItemGroupSection.pointSizeF());

    // Build "right-side" model(setting items)
    for (int iGroup = 0; iGroup < settings->groupCount(); ++iGroup) {
        const Settings::GroupIndex indexGroup(iGroup);
        const QString titleGroup = settings->groupTitle(indexGroup);
        auto listItemGroup = new QListWidgetItem(titleGroup, m_ui->listWidget_Settings);
        listItemGroup->setFont(fontItemGroupSection);
        listItemGroup->setData(ItemSettingNodeId_Role, toSettingNodeId(indexGroup));
        for (int iSection = 0; iSection < settings->sectionCount(indexGroup); ++iSection) {
            const Settings::SectionIndex indexSection(indexGroup, iSection);
            const int settingCount = settings->settingCount(indexSection);
            if (settingCount == 0)
                continue; // Skip empty section

            if (!settings->isDefaultGroupSection(indexSection)) {
                const QString titleSection =
                        tr("%1 / %2").arg(titleGroup).arg(settings->sectionTitle(indexSection));
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
            } // endfor(setting)
        } // endfor(section)
    }

    // When a setting is clicked in the "right-side" view then scroll to and select corresponding
    // tree item in the "left-side" view
    QObject::connect(m_ui->listWidget_Settings, &QListWidget::currentRowChanged, [=](int listRow) {
        auto listItem = m_ui->listWidget_Settings->item(listRow);
        const QVariant variantNodeId = listItem ? listItem->data(ItemSettingNodeId_Role) : QVariant();
        const Qt::MatchFlags matchFlags = Qt::MatchRecursive | Qt::MatchExactly;
        const QModelIndex indexFirst = treeModel->index(0, 0);
        const QModelIndexList indexList = treeModel->match(
                    indexFirst, ItemSettingNodeId_Role, variantNodeId, 1, matchFlags);
        if (!indexList.isEmpty()) {
            m_ui->treeView_GroupSections->scrollTo(indexList.front(), QAbstractItemView::PositionAtTop);
            QSignalBlocker _(m_ui->treeView_GroupSections);
            m_ui->treeView_GroupSections->selectionModel()->select(
                        indexList.front(), QItemSelectionModel::SelectCurrent);
        }
    });

    // When a group/section is clicked in the "left-side" view then scroll to the list item in
    // the "right-side" view
    QObject::connect(
                m_ui->treeView_GroupSections->selectionModel(), &QItemSelectionModel::currentChanged,
                [=](const QModelIndex& current) {
        const QAbstractItemModel* settingsModel = m_ui->listWidget_Settings->model();
        const QVariant variantNodeId = current.data(ItemSettingNodeId_Role);
        const QModelIndex indexFirst = settingsModel->index(0, 0);
        const QModelIndexList indexList = settingsModel->match(
                    indexFirst, ItemSettingNodeId_Role, variantNodeId, 1, Qt::MatchExactly);
        if (!indexList.isEmpty())
            m_ui->listWidget_Settings->scrollTo(indexList.front(), QAbstractItemView::PositionAtTop);
    });

    // Action for "Restore defaults" button
    auto btnRestoreDefaults = m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    QObject::connect(btnRestoreDefaults, &QPushButton::clicked, this, &DialogOptions::restoreDefaults);
}

DialogOptions::~DialogOptions()
{
    delete m_ui;
}

void DialogOptions::setPropertyEditorFactory(std::unique_ptr<PropertyEditorFactory> editorFactory)
{
    m_editorFactory = std::move(editorFactory);
}

QWidget* DialogOptions::createEditor(Property* property, QWidget* parentWidget) const
{
    if (!property)
        return nullptr;

    auto widget = new QWidget(parentWidget);
    auto labelName = new QLabel(QString("<b>%1</b>").arg(property->label()), widget);
    auto widgetLayout = new QVBoxLayout(widget);
    widgetLayout->addWidget(labelName);

    auto panelEditor = new QWidget(widget);
    widgetLayout->addWidget(panelEditor);
    {
        auto panelEditorLayout = new QVBoxLayout(panelEditor);
        panelEditorLayout->setContentsMargins(0, 0, 0, 10);
        if (!property->description().isEmpty()) {
            auto labelDescription = new QLabel(property->description(), panelEditor);
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
    return widget;
}

void DialogOptions::restoreDefaults()
{
    m_settings->resetAll();
    for (int i = 0; i < m_ui->listWidget_Settings->count(); ++i) {
        QListWidgetItem* listItem = m_ui->listWidget_Settings->item(i);
        QWidget* itemWidget = m_ui->listWidget_Settings->itemWidget(listItem);
        if (itemWidget) {
            auto editorWidget = itemWidget->findChild<QWidget*>(reservedPropertyEditorName);
            m_editorFactory->syncEditorWithProperty(editorWidget);
        }
    }
}

} // namespace Mayo
