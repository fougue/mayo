#include "about_dialog.h"
#include "ui_about_dialog.h"

#include <Standard_Version.hxx>

#include <gmio_core/version.h>

namespace Mayo {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_AboutDialog)
{
    m_ui->setupUi(this);

    m_ui->label_Version->setText(
                m_ui->label_Version->text().arg("0.1-dev").arg(QT_POINTER_SIZE * 8));
    m_ui->label_BuildDateTime->setText(
                m_ui->label_BuildDateTime->text().arg(__DATE__).arg(__TIME__));
    m_ui->label_Qt->setText(m_ui->label_Qt->text().arg(QT_VERSION_STR));
    m_ui->label_Occ->setText(m_ui->label_Occ->text().arg(OCC_VERSION_COMPLETE));
    m_ui->label_Gmio->setText(m_ui->label_Gmio->text().arg(GMIO_VERSION_STR));
}

AboutDialog::~AboutDialog()
{
    delete m_ui;
}

} // namespace Mayo
