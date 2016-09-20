#include "dialog_export_options.h"

#include "options.h"
#include "ui_dialog_export_options.h"

#include <gmio_stl/stl_format.h>

#include <QtCore/QByteArray>
#include <QtCore/QSettings>
#include <QtGui/QStandardItemModel>

namespace Mayo {

namespace Internal {

static const char keyExportOptionsLast[] = "ExportOptions/last";

QByteArray toByteArray(const Application::ExportOptions& opts)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << static_cast<uint32_t>(opts.stlFormat)
           << opts.stlaSolidName.c_str()
           << static_cast<uint32_t>(opts.stlaFloat32Format)
           << static_cast<uint32_t>(opts.stlaFloat32Precision);
    return bytes;
}

Application::ExportOptions fromByteArray(const QByteArray& bytes)
{
    Application::ExportOptions opts;
    QDataStream stream(bytes);
    stream >> *reinterpret_cast<uint32_t*>(&opts.stlFormat);

    char* stlaSolidName = nullptr;
    stream >> stlaSolidName;
    opts.stlaSolidName = stlaSolidName;
    delete[] stlaSolidName;

    stream >> *reinterpret_cast<uint32_t*>(&opts.stlaFloat32Format);
    stream >> *reinterpret_cast<uint32_t*>(&opts.stlaFloat32Precision);

    return opts;
}

} // namespace Internal

DialogExportOptions::DialogExportOptions(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogExportOptions)
{
    m_ui->setupUi(this);
    m_ui->comboBox_StlFormat->addItem(
                tr("ASCII"), GMIO_STL_FORMAT_ASCII);
    m_ui->comboBox_StlFormat->addItem(
                tr("Binary litte-endian"), GMIO_STL_FORMAT_BINARY_LE);
    m_ui->comboBox_StlFormat->addItem(
                tr("Binary big-endian"), GMIO_STL_FORMAT_BINARY_BE);

    QSettings settings;
    const QByteArray bytesExportOptions =
            settings.value(Internal::keyExportOptionsLast, QByteArray())
            .toByteArray();
    if (!bytesExportOptions.isEmpty()) {
        const Application::ExportOptions opts =
                Internal::fromByteArray(bytesExportOptions);
        m_ui->comboBox_StlFormat->setCurrentIndex(
                    m_ui->comboBox_StlFormat->findData(opts.stlFormat));
        m_ui->lineEdit_StlGmioAsciiSolidName->setText(
                    QString::fromLatin1(
                        QByteArray::fromStdString(opts.stlaSolidName)));
        switch (opts.stlaFloat32Format) {
        case GMIO_FLOAT_TEXT_FORMAT_DECIMAL_LOWERCASE:
        case GMIO_FLOAT_TEXT_FORMAT_DECIMAL_UPPERCASE:
            m_ui->comboBox_StlGmioAsciiFloatFormat->setCurrentIndex(0); break;
        case GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_LOWERCASE:
        case GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_UPPERCASE:
            m_ui->comboBox_StlGmioAsciiFloatFormat->setCurrentIndex(1); break;
        case GMIO_FLOAT_TEXT_FORMAT_SHORTEST_LOWERCASE:
        case GMIO_FLOAT_TEXT_FORMAT_SHORTEST_UPPERCASE:
            m_ui->comboBox_StlGmioAsciiFloatFormat->setCurrentIndex(2); break;
        }
        const bool isFormatUppercase =
                opts.stlaFloat32Format == GMIO_FLOAT_TEXT_FORMAT_DECIMAL_UPPERCASE
                || opts.stlaFloat32Format == GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_UPPERCASE
                || opts.stlaFloat32Format == GMIO_FLOAT_TEXT_FORMAT_SHORTEST_UPPERCASE;
        m_ui->checkBox_StlGmioAsciiFloatFormatUppercase->setChecked(isFormatUppercase);
        m_ui->spinBox_StlGmioAsciiFloatPrecision->setValue(opts.stlaFloat32Precision);
    }
}

DialogExportOptions::~DialogExportOptions()
{
    delete m_ui;
}

Application::PartFormat DialogExportOptions::partFormat() const
{
    return m_partFormat;
}

void DialogExportOptions::setPartFormat(Application::PartFormat format)
{
    m_partFormat = format;
    if (format == Application::PartFormat::Stl) {
        const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
        m_ui->widget_StlGmio->setEnabled(lib == Options::StlIoLibrary::Gmio);
        auto comboBoxStlFormatModel =
                qobject_cast<QStandardItemModel*>(m_ui->comboBox_StlFormat->model());
        QStandardItem* itemBinaryBigEndian = comboBoxStlFormatModel->item(2);
        itemBinaryBigEndian->setEnabled(lib == Options::StlIoLibrary::Gmio);
        if (lib == Options::StlIoLibrary::Gmio) {
            QObject::connect(
                        m_ui->comboBox_StlFormat,
                        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        [=](int index) {
                m_ui->widget_StlGmioAscii->setEnabled(index == 0);
            });
            m_ui->widget_StlGmioAscii->setEnabled(
                        m_ui->comboBox_StlFormat->currentData().toInt()
                        == GMIO_STL_FORMAT_ASCII);
        }
        else {
            if (m_ui->comboBox_StlFormat->currentData().toInt()
                    == GMIO_STL_FORMAT_BINARY_BE)
            {
                const int comboBoxBinLeId =
                        m_ui->comboBox_StlFormat->findData(GMIO_STL_FORMAT_BINARY_LE);
                m_ui->comboBox_StlFormat->setCurrentIndex(comboBoxBinLeId);
            }
        }
    }
}

Application::ExportOptions DialogExportOptions::currentExportOptions() const
{
    const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
    Application::ExportOptions options;
    options.stlFormat = static_cast<gmio_stl_format>(
                m_ui->comboBox_StlFormat->currentData().toInt());
    if (lib == Options::StlIoLibrary::Gmio) {
        options.stlaSolidName =
                m_ui->lineEdit_StlGmioAsciiSolidName->text().toLatin1().toStdString();
        const int comboBoxFloatFormatId =
                m_ui->comboBox_StlGmioAsciiFloatFormat->currentIndex();
        const bool floatFormatUppercase =
                m_ui->checkBox_StlGmioAsciiFloatFormatUppercase->isChecked();
        if (comboBoxFloatFormatId == 0) {
            options.stlaFloat32Format = floatFormatUppercase ?
                        GMIO_FLOAT_TEXT_FORMAT_DECIMAL_UPPERCASE :
                        GMIO_FLOAT_TEXT_FORMAT_DECIMAL_LOWERCASE;
        }
        else if (comboBoxFloatFormatId == 1) {
            options.stlaFloat32Format = floatFormatUppercase ?
                        GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_UPPERCASE :
                        GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_LOWERCASE;
        }
        else if (comboBoxFloatFormatId == 2) {
            options.stlaFloat32Format = floatFormatUppercase ?
                        GMIO_FLOAT_TEXT_FORMAT_SHORTEST_LOWERCASE :
                        GMIO_FLOAT_TEXT_FORMAT_SHORTEST_UPPERCASE;
        }
        options.stlaFloat32Precision =
                m_ui->spinBox_StlGmioAsciiFloatPrecision->value();
    }
    return options;
}

void DialogExportOptions::accept()
{
    QSettings settings;
    settings.setValue(
                Internal::keyExportOptionsLast,
                Internal::toByteArray(this->currentExportOptions()));
    QDialog::accept();
}

} // namespace Mayo
