/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_about.h"

#include "ui_dialog_about.h"
#include "version.h"
#include <Standard_Version.hxx>
#ifdef HAVE_GMIO
#  include <gmio_core/version.h>
#endif

namespace Mayo {

DialogAbout::DialogAbout(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogAbout)
{
    m_ui->setupUi(this);
    m_ui->label_AppByOrg->setText(
                tr("%1 By %2").arg(QApplication::applicationName(),
                                   QApplication::organizationName()));

    m_ui->label_Version->setText(
                m_ui->label_Version->text().arg(strVersion).arg(QT_POINTER_SIZE * 8));
    m_ui->label_BuildDateTime->setText(
                m_ui->label_BuildDateTime->text().arg(__DATE__).arg(__TIME__));
    m_ui->label_Qt->setText(m_ui->label_Qt->text().arg(QT_VERSION_STR));
    m_ui->label_Occ->setText(m_ui->label_Occ->text().arg(OCC_VERSION_COMPLETE));
#ifdef HAVE_GMIO
    m_ui->label_Gmio->setText(m_ui->label_Gmio->text().arg(GMIO_VERSION_STR));
#else
    m_ui->label_Gmio->hide();
#endif
}

DialogAbout::~DialogAbout()
{
    delete m_ui;
}

} // namespace Mayo
