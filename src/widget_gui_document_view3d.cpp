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

#include "widget_gui_document_view3d.h"

#include "gui_document.h"
#include "widget_occ_view.h"
#include "qt_occ_view_controller.h"
#include "fougtools/qttools/gui/qwidget_utils.h"

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QToolButton>

#include <V3d_TypeOfOrientation.hxx>

namespace Mayo {

namespace Internal {

static QToolButton* createViewBtn(
        QWidget* parent, const QString& imageFile, const QString& tooltip)
{
    auto btn = new QToolButton(parent);
    btn->setIcon(QIcon(QString(":/images/%1.png").arg(imageFile)));
    btn->setIconSize(QSize(16, 16));
    btn->setFixedSize(24, 24);
    btn->setToolTip(tooltip);
    return btn;
}

static void connectViewProjBtn(
        QToolButton* btn, WidgetOccView* view, V3d_TypeOfOrientation proj)
{
    QObject::connect(
                btn, &QAbstractButton::clicked,
                [=]{
        view->occV3dView()->SetProj(proj);
        view->fitAll();
    });
}

} // namespace Internal

WidgetGuiDocumentView3d::WidgetGuiDocumentView3d(GuiDocument* guiDoc, QWidget *parent)
    : QWidget(parent),
      m_guiDoc(guiDoc),
      m_qtOccView(new WidgetOccView(this))
{
    new QtOccViewController(m_qtOccView);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_qtOccView);
    this->setLayout(layout);

    auto btnFitAll = Internal::createViewBtn(this, "fit_all", tr("Fit All"));
    auto btnViewIso = Internal::createViewBtn(this, "view_axo", tr("Isometric"));
    auto btnViewBack = Internal::createViewBtn(this, "view_back", tr("Back"));
    auto btnViewFront = Internal::createViewBtn(this, "view_front", tr("Front"));
    auto btnViewLeft = Internal::createViewBtn(this, "view_left", tr("Left"));
    auto btnViewRight = Internal::createViewBtn(this, "view_right", tr("Right"));
    auto btnViewTop = Internal::createViewBtn(this, "view_top", tr("Top"));
    auto btnViewBottom = Internal::createViewBtn(this, "view_bottom", tr("Bottom"));
    btnFitAll->move(0, 0);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewIso, btnFitAll);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBack, btnViewIso);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewFront, btnViewBack);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewLeft, btnViewFront);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewRight, btnViewLeft);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewTop, btnViewRight);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBottom, btnViewTop);

    Internal::connectViewProjBtn(btnViewIso, m_qtOccView, V3d_XposYnegZpos);
    Internal::connectViewProjBtn(btnViewBack, m_qtOccView, V3d_Xneg);
    Internal::connectViewProjBtn(btnViewFront, m_qtOccView, V3d_Xpos);
    Internal::connectViewProjBtn(btnViewLeft, m_qtOccView, V3d_Ypos);
    Internal::connectViewProjBtn(btnViewRight, m_qtOccView, V3d_Yneg);
    Internal::connectViewProjBtn(btnViewTop, m_qtOccView, V3d_Zpos);
    Internal::connectViewProjBtn(btnViewBottom, m_qtOccView, V3d_Zneg);
    QObject::connect(
                btnFitAll, &QAbstractButton::clicked,
                m_qtOccView, &WidgetOccView::fitAll);
}

GuiDocument *WidgetGuiDocumentView3d::guiDocument() const
{
    return m_guiDoc;
}

WidgetOccView *WidgetGuiDocumentView3d::widgetOccView() const
{
    return m_qtOccView;
}

} // namespace Mayo
