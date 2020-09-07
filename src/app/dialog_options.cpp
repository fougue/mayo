/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_options.h"

#include "../base/application.h"
#include "../base/property_enumeration.h"
#include "../base/occt_enums.h"
#include "../base/settings.h"
#include "../base/unit_system.h"
#include "../base/settings_keys.h"
#include "ui_dialog_options.h"

#include <fougtools/qttools/gui/qwidget_utils.h>
#include <fougtools/occtools/qt_utils.h>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QColorDialog>

namespace Mayo {

namespace Internal {

static QPixmap colorPixmap(const QColor& color)
{
    QPixmap pix(16, 16);
    pix.fill(color);
    return pix;
}

} // namespace Internal

DialogOptions::DialogOptions(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogOptions)
{
    m_ui->setupUi(this);
#ifndef HAVE_GMIO
    m_ui->groupBox_StlIo->hide();
    this->adjustSize();
#endif

    auto settings = Application::instance()->settings();

    // STL import/export
    auto btnGrp_stlIoLib = new QButtonGroup(this);
    btnGrp_stlIoLib->addButton(m_ui->radioBtn_UseGmio);
    btnGrp_stlIoLib->addButton(m_ui->radioBtn_UseOcc);

//    const auto lib = settings->valueAsEnum<Application::StlIoLibrary>(Keys::Base_StlIoLibrary);
//    m_ui->radioBtn_UseGmio->setChecked(lib == Application::StlIoLibrary::Gmio);
//    m_ui->radioBtn_UseOcc->setChecked(lib == Application::StlIoLibrary::OpenCascade);

    // Mesh defaults
    m_meshDefaultColor = settings->valueAs<QColor>(Keys::Gpx_MeshDefaultColor);
    m_ui->toolBtn_MeshDefaultColor->setIcon(Internal::colorPixmap(m_meshDefaultColor));
    QObject::connect(m_ui->toolBtn_MeshDefaultColor, &QAbstractButton::clicked, [=]{
        const QColor startColor = settings->valueAs<QColor>(Keys::Gpx_MeshDefaultColor);
        this->chooseColor(startColor, [=](QColor color) {
            m_ui->toolBtn_MeshDefaultColor->setIcon(Internal::colorPixmap(color));
            m_meshDefaultColor = color;
        });
    });
    for (const Enumeration::Item& m : OcctEnums::Graphic3d_NameOfMaterial().items())
        m_ui->comboBox_MeshDefaultMaterial->addItem(m.text, m.value);

    m_ui->comboBox_MeshDefaultMaterial->setCurrentIndex(
                m_ui->comboBox_MeshDefaultMaterial->findData(
                    settings->valueAsEnum<Graphic3d_NameOfMaterial>(Keys::Gpx_MeshDefaultMaterial)));
    m_ui->checkBox_MeshShowEdges->setChecked(settings->valueAs<bool>(Keys::Gpx_MeshDefaultShowEdges));
    m_ui->checkBox_MeshShowNodes->setChecked(settings->valueAs<bool>(Keys::Gpx_MeshDefaultShowNodes));

    // Clip planes
    m_ui->checkBox_Capping->setChecked(settings->valueAs<bool>(Keys::Gui_ClipPlaneCappingOn));
    for (const Enumeration::Item& m : OcctEnums::Aspect_HatchStyle().items())
        m_ui->comboBox_CappingHatch->addItem(m.name, m.value);

    m_ui->comboBox_CappingHatch->setCurrentIndex(
                m_ui->comboBox_CappingHatch->findData(
                    settings->valueAsEnum<Aspect_HatchStyle>(Keys::Gui_ClipPlaneCappingHatch)));
    QObject::connect(
                m_ui->checkBox_Capping, &QAbstractButton::clicked,
                m_ui->widget_CappingHatch, &QWidget::setEnabled);
    m_ui->widget_CappingHatch->setEnabled(m_ui->checkBox_Capping->isChecked());

    // Units
    m_ui->comboBox_UnitSystem->addItem(tr("SI"), UnitSystem::SI);
    m_ui->comboBox_UnitSystem->addItem(tr("Imperial UK"), UnitSystem::ImperialUK);
    m_ui->comboBox_UnitSystem->setCurrentIndex(
                m_ui->comboBox_UnitSystem->findData(settings->unitSystemSchema()));
    m_ui->spinBox_Decimals->setValue(settings->unitSystemDecimals());
}

DialogOptions::~DialogOptions()
{
    delete m_ui;
}

void DialogOptions::accept()
{
    auto settings = Application::instance()->settings();

//    // STL import/export
//    if (m_ui->radioBtn_UseGmio->isChecked())
//        settings->setValue(Keys::Base_StlIoLibrary, int(Application::StlIoLibrary::Gmio));
//    else if (m_ui->radioBtn_UseOcc->isChecked())
//        settings->setValue(Keys::Base_StlIoLibrary, int(Application::StlIoLibrary::OpenCascade));

    // Mesh defaults
    settings->setValue(Keys::Gpx_MeshDefaultColor, m_meshDefaultColor);
    settings->setValue(Keys::Gpx_MeshDefaultMaterial, m_ui->comboBox_MeshDefaultMaterial->currentData());
    settings->setValue(Keys::Gpx_MeshDefaultShowEdges, m_ui->checkBox_MeshShowEdges->isChecked());
    settings->setValue(Keys::Gpx_MeshDefaultShowNodes, m_ui->checkBox_MeshShowNodes->isChecked());

    // Clip planes
    settings->setValue(Keys::Gui_ClipPlaneCappingOn, m_ui->checkBox_Capping->isChecked());
    settings->setValue(Keys::Gui_ClipPlaneCappingHatch, m_ui->comboBox_CappingHatch->currentData());

    // Units
    settings->setValue(Keys::Base_UnitSystemSchema, m_ui->comboBox_UnitSystem->currentData());
    settings->setValue(Keys::Base_UnitSystemDecimals, m_ui->spinBox_Decimals->value());

    QDialog::accept();
}

void DialogOptions::chooseColor(
        const QColor& currentColor,
        const std::function<void(QColor)>& continuation)
{
    auto dlg = new QColorDialog(this);
    dlg->setCurrentColor(currentColor);
    QObject::connect(dlg, &QDialog::finished, [=](int result) {
        if (result == QDialog::Accepted)
            continuation(dlg->selectedColor());
    });
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

} // namespace Mayo
