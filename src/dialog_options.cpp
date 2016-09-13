#include "dialog_options.h"

#include "options.h"
#include "property_enumeration.h"
#include "ui_dialog_options.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/occtools/qt_utils.h"

#include <QtWidgets/QColorDialog>

namespace Mayo {

namespace Internal {

static QPixmap colorPixmap(const QColor& color)
{
    QPixmap pix(24, 24);
    pix.fill(color);
    return pix;
}

} // namespace Internal

DialogOptions::DialogOptions(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogOptions)
{
    m_ui->setupUi(this);

    const Options* opts = Options::instance();
    const Options::StlIoLibrary lib = opts->stlIoLibrary();
    if (lib == Options::StlIoLibrary::Gmio)
        m_ui->radioBtn_UseGmio->setChecked(true);
    else if (lib == Options::StlIoLibrary::OpenCascade)
        m_ui->radioBtn_UseOcc->setChecked(true);

    m_ui->toolBtn_BRepShapeDefaultColor->setIcon(
                Internal::colorPixmap(opts->brepShapeDefaultColor()));
    m_ui->toolBtn_MeshDefaultColor->setIcon(
                Internal::colorPixmap(opts->meshDefaultColor()));
    m_brepShapeDefaultColor = opts->brepShapeDefaultColor();
    m_meshDefaultColor = opts->meshDefaultColor();
    QObject::connect(
                m_ui->toolBtn_BRepShapeDefaultColor, &QAbstractButton::clicked,
                [=] {
        this->chooseColor(opts->brepShapeDefaultColor(),
                          m_ui->toolBtn_BRepShapeDefaultColor,
                          &m_brepShapeDefaultColor);
    } );
    QObject::connect(
                m_ui->toolBtn_MeshDefaultColor, &QAbstractButton::clicked,
                [=] {
        this->chooseColor(opts->meshDefaultColor(),
                          m_ui->toolBtn_MeshDefaultColor,
                          &m_meshDefaultColor);
    } );

    const auto& vecGpxMaterialMapping =
            Mayo::enum_Graphic3dNameOfMaterial().mappings();
    for (const Enumeration::Mapping& mapping : vecGpxMaterialMapping) {
        m_ui->comboBox_BRepShapeDefaultMaterial->addItem(
                    mapping.string, mapping.value);
        m_ui->comboBox_MeshDefaultMaterial->addItem(
                    mapping.string, mapping.value);
    }
    m_ui->comboBox_BRepShapeDefaultMaterial->setCurrentIndex(
                m_ui->comboBox_BRepShapeDefaultMaterial->findData(
                    static_cast<int>(opts->brepShapeDefaultMaterial())));
    m_ui->comboBox_MeshDefaultMaterial->setCurrentIndex(
                m_ui->comboBox_MeshDefaultMaterial->findData(
                    static_cast<int>(opts->meshDefaultMaterial())));

    m_ui->checkBox_MeshShowEdges->setChecked(opts->meshDefaultShowEdges());
    m_ui->checkBox_MeshShowNodes->setChecked(opts->meshDefaultShowNodes());
}

DialogOptions::~DialogOptions()
{
    delete m_ui;
}

void DialogOptions::accept()
{
    Options* opts = Options::instance();

    if (m_ui->radioBtn_UseGmio->isChecked())
        opts->setStlIoLibrary(Options::StlIoLibrary::Gmio);
    else if (m_ui->radioBtn_UseOcc->isChecked())
        opts->setStlIoLibrary(Options::StlIoLibrary::OpenCascade);

    opts->setBrepShapeDefaultColor(m_brepShapeDefaultColor);
    opts->setMeshDefaultColor(m_meshDefaultColor);

    opts->setBrepShapeDefaultMaterial(
                static_cast<Graphic3d_NameOfMaterial>(
                    m_ui->comboBox_BRepShapeDefaultMaterial->currentData().toInt()));
    opts->setMeshDefaultMaterial(
                static_cast<Graphic3d_NameOfMaterial>(
                    m_ui->comboBox_MeshDefaultMaterial->currentData().toInt()));

    opts->setMeshDefaultShowEdges(m_ui->checkBox_MeshShowEdges->isChecked());
    opts->setMeshDefaultShowNodes(m_ui->checkBox_MeshShowNodes->isChecked());

    QDialog::accept();
}

void DialogOptions::chooseColor(
        const QColor& currentColor, QToolButton* targetBtn, QColor* targetColor)
{
    auto dlg = new QColorDialog(this);
    dlg->setCurrentColor(currentColor);
    QObject::connect(
                dlg, &QDialog::finished,
                [=](int result) {
        if (result == QDialog::Accepted) {
            targetBtn->setIcon(Internal::colorPixmap(dlg->selectedColor()));
            *targetColor = dlg->selectedColor();
        }
    } );
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

} // namespace Mayo
