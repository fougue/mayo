/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_about.h"

#include "ui_dialog_about.h"
#include "../qtcommon/qstring_conv.h"
#include <common/mayo_version.h>

#include <Standard_Version.hxx>

namespace Mayo {

DialogAbout::DialogAbout(QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogAbout)
{
    m_ui->setupUi(this);
    m_ui->label_AppByOrg->setText(
        tr("%1 By %2").arg(QApplication::applicationName(), QApplication::organizationName())
    );

    const QString strVersionFull =
        QString("%1 commit:%2 revnum:%3")
        .arg(strVersion).arg(strVersionCommitId).arg(versionRevisionNumber);
    m_ui->label_Version->setText(m_ui->label_Version->text().arg(strVersionFull).arg(QT_POINTER_SIZE * 8));
    m_ui->label_BuildDateTime->setText(m_ui->label_BuildDateTime->text().arg(__DATE__, __TIME__));
    m_ui->label_Qt->setText(m_ui->label_Qt->text().arg(QT_VERSION_STR));
    m_ui->label_Occ->setText(m_ui->label_Occ->text().arg(OCC_VERSION_COMPLETE));
}

DialogAbout::~DialogAbout()
{
    delete m_ui;
}

void DialogAbout::addLibraryInfo(std::string_view libName, std::string_view libVersion)
{
    auto label = new QLabel(this);
    label->setText(tr("%1 %2").arg(to_QString(libName), to_QString(libVersion)));
    m_ui->layout_Infos->addWidget(label);;
}

} // namespace Mayo
