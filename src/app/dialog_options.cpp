/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_options.h"

#include "../base/settings.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "ui_dialog_options.h"
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

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

QString stringYesNo(bool on)
{
    return on ? DialogOptions::tr("Yes") : DialogOptions::tr("No");
}

QLabel* createPropertyDescriptionLabel(Property* property, QWidget* parentWidget)
{
    QLabel* label = new QLabel(parentWidget);
    if (!property->description().isEmpty()) {
        label->setText(property->description());
        label->setMinimumWidth(600);
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignLeft);
    }

//    if (parentWidget && parentWidget->layout())
//        parentWidget->layout()->addWidget(label);

    return label;
}

QBoxLayout* createPropertyHBoxLayout(QWidget* widget)
{
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 10);
    return layout;
}

QBoxLayout* createPropertyVBoxLayout(QWidget* widget)
{
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 10);
    return layout;
}

QWidget* createPropertyEditor(PropertyBool* property, QWidget* parentWidget)
{
    auto panel = new QWidget(parentWidget);
    auto editor = new QCheckBox(panel);
    auto label = createPropertyDescriptionLabel(property, panel);

    auto panelLayout = createPropertyHBoxLayout(panel);
    panelLayout->addWidget(editor);
    panelLayout->addWidget(label);
    panelLayout->itemAt(0)->setAlignment(Qt::AlignTop);
    panelLayout->itemAt(1)->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    panelLayout->addSpacerItem(new QSpacerItem(20, 5, QSizePolicy::Expanding));

    editor->setChecked(property->value());
    if (!property->description().isEmpty())
        label->setText(property->description());
    else
        label->setText(stringYesNo(property->value()));

    QObject::connect(editor, &QCheckBox::toggled, [=](bool on) {
        property->setValue(on);
        editor->setChecked(on);
        if (property->description().isEmpty())
            label->setText(stringYesNo(on));
    });
    return panel;
}

static QWidget* createPropertyEditor(PropertyInt* property, QWidget* parentWidget)
{
    auto panel = new QWidget(parentWidget);

    auto editor = new QSpinBox(panel);
    editor->setMaximumWidth(150);
    if (property->constraintsEnabled()) {
        editor->setRange(property->minimum(), property->maximum());
        editor->setSingleStep(property->singleStep());
    }

    auto panelLayout = createPropertyVBoxLayout(panel);
    panelLayout->addWidget(createPropertyDescriptionLabel(property, panel));
    panelLayout->addWidget(editor);

    editor->setValue(property->value());
    QObject::connect(editor, qOverload<int>(&QSpinBox::valueChanged), [=](int val) {
        property->setValue(val);
    });

    return panel;
}

QWidget* createPropertyEditor(PropertyEnumeration* property, QWidget* parentWidget)
{
    auto panel = new QWidget(parentWidget);

    auto panelLayout = createPropertyVBoxLayout(panel);
    if (!property->description().isEmpty()) {
        auto label = new QLabel(property->description(), panel);
        label->setMinimumWidth(600);
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignLeft);
        panelLayout->addWidget(label);
        //panelLayout->itemAt(1)->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    }

    auto editor = new QComboBox(panel);
    editor->setMaximumWidth(250);
    panelLayout->addWidget(editor);

    const Enumeration* enumDef = property->enumeration();
    if (!enumDef)
        return panel;

    for (const Enumeration::Item& enumItem : enumDef->items())
        editor->addItem(enumItem.name.tr(), enumItem.value);

    editor->setCurrentIndex(editor->findData(property->value()));
    QObject::connect(editor, qOverload<int>(&QComboBox::activated), [=](int index) {
        property->setValue(editor->itemData(index).toInt());
        editor->setCurrentIndex(editor->findData(property->value()));
    });
    return panel;
}

QWidget* createPropertyEditor(Property* property, QWidget* parentWidget)
{
    if (!property)
        return nullptr;

    auto widget = new QWidget(parentWidget);
    auto labelProperty = new QLabel(QString("<b>%1</b>").arg(property->label()), widget);
    auto widgetLayout = new QVBoxLayout(widget);
    widgetLayout->addWidget(labelProperty);

    QWidget* editor = nullptr;
    const char* propertyTypeName = property->dynTypeName();
    if (propertyTypeName == PropertyBool::TypeName)
        editor = createPropertyEditor(static_cast<PropertyBool*>(property), widget);
    if (propertyTypeName == PropertyInt::TypeName)
        editor = createPropertyEditor(static_cast<PropertyInt*>(property), widget);
    if (propertyTypeName == PropertyEnumeration::TypeName)
        editor = createPropertyEditor(static_cast<PropertyEnumeration*>(property), widget);

    if (editor)
        widgetLayout->addWidget(editor);

    widgetLayout->setSizeConstraint(QLayout::SetMaximumSize);
    return widget;
}

} // namespace

DialogOptions::DialogOptions(Settings* settings, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogOptions)
{
    m_ui->setupUi(this);

    auto treeModel = createGroupSectionModel(settings, this);
    m_ui->treeView_GroupSections->setModel(treeModel);
    m_ui->treeView_GroupSections->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QFont fontItemGroupSection = m_ui->listWidget_Settings->font();
    fontItemGroupSection.setBold(true);
    fontItemGroupSection.setPointSizeF(1.7 * fontItemGroupSection.pointSizeF());

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

                auto widget = createPropertyEditor(property, nullptr);
                auto listItemSetting = new QListWidgetItem;
                listItemSetting->setData(ItemSettingNodeId_Role, toSettingNodeId(indexSection));
                listItemSetting->setSizeHint(widget->sizeHint());
                m_ui->listWidget_Settings->addItem(listItemSetting);
                m_ui->listWidget_Settings->setItemWidget(listItemSetting, widget);
            } // endfor(setting)
        } // endfor(section)
    }

    QObject::connect(
                m_ui->treeView_GroupSections->selectionModel(), &QItemSelectionModel::currentChanged,
                [=](const QModelIndex& current) {
        if (current.isValid()) {
            const SettingNodeId nodeId = treeModel->data(current, ItemSettingNodeId_Role).toUInt();
            for (int i = 0; i < m_ui->listWidget_Settings->count(); ++i) {
                const QListWidgetItem* item = m_ui->listWidget_Settings->item(i);
                const QVariant variantSettingNodeId = item->data(ItemSettingNodeId_Role);
                if (variantSettingNodeId.isValid() && variantSettingNodeId.toUInt() == nodeId) {
                    m_ui->listWidget_Settings->scrollToItem(item, QListWidget::PositionAtTop);
                    return;
                }
            }
        }
    });

#if 0
    for (int iGroup = 0; iGroup < settings->groupCount(); ++iGroup) {
        const Settings::GroupIndex indexGroup(iGroup);
        const QString titleGroup = settings->groupTitle(indexGroup);
        for (int iSection = 0; iSection < settings->sectionCount(indexGroup); ++iSection) {
            const Settings::SectionIndex indexSection(indexGroup, iSection);
            const int settingCount = settings->settingCount(indexSection);
            if (settingCount == 0)
                continue; // Skip empty section

            const QString titleSection = settings->sectionTitle(indexSection);
            const QString uiTitleSection =
                    !settings->isDefaultGroupSection(indexSection) ?
                        tr("/%1").arg(titleSection) :
                        QString();
            WidgetPropertiesEditor::Group* group = m_ui->widgetEditor->addGroup(titleGroup + uiTitleSection);
            for (int iSetting = 0; iSetting < settingCount; ++iSetting) {
                Property* property = settings->property(Settings::SettingIndex(indexSection, iSetting));
                if (property->isUserVisible())
                    m_ui->widgetEditor->editProperty(property, group);
            }
        }
    }
#endif

    auto btnResetAll = m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    QObject::connect(btnResetAll, &QPushButton::clicked, settings, &Settings::resetAll);
}

DialogOptions::~DialogOptions()
{
    delete m_ui;
}

} // namespace Mayo
