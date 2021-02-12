/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_save_image_view.h"
#include "ui_dialog_save_image_view.h"
#include "widgets_utils.h"
#include "../graphics/graphics_utils.h"

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
                     int(occImg.Width()),
                     int(occImg.Height()),
                     int(occImg.SizeRowBytes()),
                     QImage::Format_RGBA8888);
    return img;
}

} // namespace Internal

DialogSaveImageView::DialogSaveImageView(const Handle_V3d_View& view, QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogSaveImageView),
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
                this, &DialogSaveImageView::saveFile);
    QObject::connect(
                copyBtn, &QAbstractButton::clicked,
                this, &DialogSaveImageView::clipboardCopy);
    QObject::connect(
                previewBtn, &QAbstractButton::clicked,
                this, &DialogSaveImageView::preview);

    m_ui->edit_Width->setValue(GraphicsUtils::AspectWindow_width(view->Window()));
    m_ui->edit_Height->setValue(GraphicsUtils::AspectWindow_height(view->Window()));
}

DialogSaveImageView::~DialogSaveImageView()
{
    delete m_ui;
}

void DialogSaveImageView::saveFile()
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
        bool saveOk = this->createImageView(&occPix);
        if (saveOk) {
            const QImage img = Internal::qtImageTemp(occPix);
            saveOk = img.save(fileName, format);
        }
        if (!saveOk) {
            WidgetsUtils::asyncMsgBoxCritical(
                        this, tr("Error"), tr("Failed to save image '%1'").arg(fileName));
        }
    }
}

void DialogSaveImageView::clipboardCopy()
{
    Image_PixMap occPix;
    if (this->createImageView(&occPix)) {
        const QImage img = Internal::qtImageTemp(occPix).copy();
        QApplication::clipboard()->setImage(img, QClipboard::Clipboard);
    }
}

void DialogSaveImageView::preview()
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

bool DialogSaveImageView::createImageView(Image_PixMap* img) const
{
    img->SetTopDown(true);
    const bool ok =
            m_view->ToPixMap(
                *img,
                m_ui->edit_Width->value(),
                m_ui->edit_Height->value(),
                Graphic3d_BT_RGBA,
                m_ui->checkBox_KeepRatio->isChecked());
    return ok;
}

} // namespace Mayo
