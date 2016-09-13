#include "save_image_view_dialog.h"

#include "qt_occ_view.h"
#include "ui_save_image_view_dialog.h"
#include "fougtools/qttools/gui/qwidget_utils.h"

#include <QtCore/QHash>
#include <QtGui/QClipboard>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>

namespace Mayo {

namespace Internal {

static QImage qtImageTemp(const Image_PixMap& occImg)
{
    const QImage img(occImg.Data(),
                     occImg.Width(),
                     occImg.Height(),
                     static_cast<int>(occImg.SizeRowBytes()),
                     QImage::Format_ARGB32);
    return img;
}

} // namespace Internal

SaveImageViewDialog::SaveImageViewDialog(const QtOccView* view, QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_SaveImageViewDialog),
      m_view(view)
{
    m_ui->setupUi(this);

    auto saveBtn = new QPushButton(tr("Save"), this);
    auto copyBtn = new QPushButton(tr("Copy"), this);
    auto previewBtn = new QPushButton(tr("Preview"), this);
    m_ui->buttonBox->addButton(saveBtn, QDialogButtonBox::ActionRole);
    m_ui->buttonBox->addButton(copyBtn, QDialogButtonBox::ActionRole);
    m_ui->buttonBox->addButton(previewBtn, QDialogButtonBox::ActionRole);
    QObject::connect(
                saveBtn, &QAbstractButton::clicked,
                this, &SaveImageViewDialog::saveFile);
    QObject::connect(
                copyBtn, &QAbstractButton::clicked,
                this, &SaveImageViewDialog::clipboardCopy);
    QObject::connect(
                previewBtn, &QAbstractButton::clicked,
                this, &SaveImageViewDialog::preview);

    m_ui->edit_Width->setValue(view->geometry().width());
    m_ui->edit_Height->setValue(view->geometry().height());
}

SaveImageViewDialog::~SaveImageViewDialog()
{
    delete m_ui;
}

void SaveImageViewDialog::saveFile()
{
    QHash<QString, QByteArray> mapFilterFormat;
    QStringList listFormat;
    foreach (const QByteArray& name, QImageWriter::supportedImageFormats()) {
        const QString strName = QString::fromLatin1(name);
        listFormat.append(tr("%1 files(*.%2)")
                          .arg(strName.toUpper(), strName.toLower()));
        mapFilterFormat.insert(listFormat.back(), name);
    }

    QString selectedFormat;
    const QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Select image file"),
                QString(),
                listFormat.join(QLatin1String(";;")),
                &selectedFormat);
    if (!fileName.isEmpty()) {
        auto itFound = mapFilterFormat.find(selectedFormat);
        const char* format =
                itFound != mapFilterFormat.cend() ?
                    itFound.value().constData() :
                    nullptr;

        Image_PixMap occPix;
        bool saveOk = false;
        if (saveOk = this->createImageView(&occPix)) {
            const QImage img = Internal::qtImageTemp(occPix);
            saveOk = img.save(fileName, format);
        }
        if (!saveOk) {
            qtgui::QWidgetUtils::asyncMsgBoxCritical(
                        this,
                        tr("Error"),
                        tr("Failed to save image '%1'").arg(fileName));
        }
    }
}

void SaveImageViewDialog::clipboardCopy()
{
    Image_PixMap occPix;
    if (this->createImageView(&occPix)) {
        const QImage img = Internal::qtImageTemp(occPix).copy();
        QApplication::clipboard()->setImage(img, QClipboard::Clipboard);
    }
}

void SaveImageViewDialog::preview()
{
    Image_PixMap occPix;
    if (this->createImageView(&occPix)) {
        const QImage img = Internal::qtImageTemp(occPix);
        auto label = new QLabel(this, Qt::Dialog);
        label->setPixmap(QPixmap::fromImage(img));
        label->setWindowTitle(
                    tr("%1x%2 %3")
                    .arg(img.width())
                    .arg(img.height())
                    .arg(m_ui->checkBox_KeepRatio->isChecked() ?
                             tr("Keep ratio") : tr("Free ratio")));
        label->setAttribute(Qt::WA_DeleteOnClose, true);
        label->show();
    }
}

bool SaveImageViewDialog::createImageView(Image_PixMap* img) const
{
    img->SetTopDown(true);
    const Standard_Boolean ok = m_view->occV3dView()->ToPixMap(
                *img,
                m_ui->edit_Width->value(),
                m_ui->edit_Height->value(),
                Graphic3d_BT_RGBA,
                m_ui->checkBox_KeepRatio->isChecked());
    return ok == Standard_True;
}

} // namespace Mayo
