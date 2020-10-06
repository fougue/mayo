/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_options.h"

#include "../base/settings.h"
#include "ui_dialog_options.h"
#include <QtWidgets/QPushButton>

namespace Mayo {

DialogOptions::DialogOptions(Settings* settings, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogOptions)
{
    m_ui->setupUi(this);

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

    auto btnResetAll = m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    QObject::connect(btnResetAll, &QPushButton::clicked, settings, &Settings::resetAll);
}

DialogOptions::~DialogOptions()
{
    delete m_ui;
}

} // namespace Mayo
