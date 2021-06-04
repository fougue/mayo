/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_explode_assembly.h"
#include "ui_widget_explode_assembly.h"
#include "../gui/gui_document.h"

#include <QtCore/QSignalBlocker>

namespace Mayo {

WidgetExplodeAssembly::WidgetExplodeAssembly(GuiDocument* guiDoc, QWidget* parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetExplodeAssembly),
      m_guiDoc(guiDoc)
{
    m_ui->setupUi(this);

    QObject::connect(m_ui->slider_Factor, &QSlider::valueChanged, this, [=](int pct) {
        QSignalBlocker sigBlock(m_ui->edit_Factor);
        m_ui->edit_Factor->setValue(pct);
        m_guiDoc->setExplodingFactor(pct / 100.);
    });
    QObject::connect(m_ui->edit_Factor, qOverload<int>(&QSpinBox::valueChanged), this, [=](int pct) {
        QSignalBlocker sigBlock(m_ui->slider_Factor);
        m_ui->slider_Factor->setValue(pct);
        m_guiDoc->setExplodingFactor(pct / 100.);
    });
}

WidgetExplodeAssembly::~WidgetExplodeAssembly()
{
    delete m_ui;
}

} // namespace Mayo
