/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "dialog_save_image_view.h"

#include "widget_occ_view.h"
#include "ui_dialog_save_image_view.h"
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
                     static_cast<int>(occImg.Width()),
                     static_cast<int>(occImg.Height()),
                     static_cast<int>(occImg.SizeRowBytes()),
                     QImage::Format_RGBA8888);
    return img;
}

} // namespace Internal

DialogSaveImageView::DialogSaveImageView(const WidgetOccView* view, QWidget *parent)
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

    m_ui->edit_Width->setValue(view->geometry().width());
    m_ui->edit_Height->setValue(view->geometry().height());
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
            m_view->occV3dView()->ToPixMap(
                *img,
                m_ui->edit_Width->value(),
                m_ui->edit_Height->value(),
                Graphic3d_BT_RGBA,
                m_ui->checkBox_KeepRatio->isChecked());
    return ok;
}

} // namespace Mayo
