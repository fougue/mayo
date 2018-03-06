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

#include "button_view3d.h"
#include "gui_document.h"
#include "widget_occ_view.h"
#include "qt_occ_view_controller.h"
#include "widget_clip_planes.h"
#include "fougtools/qttools/gui/qwidget_utils.h"

#include <QtGui/QPainter>
#include <QtWidgets/QBoxLayout>

#include <V3d_TypeOfOrientation.hxx>

namespace Mayo {

namespace Internal {

static ButtonView3d* createViewBtn(
        QWidget* parent, QString imageFile, QString tooltip)
{
    auto btn = new ButtonView3d(parent);
    imageFile = imageFile.contains('.') ? imageFile : (imageFile + ".png");
    btn->setIcon(QIcon(QString(":/images/%1").arg(imageFile)));
    btn->setIconSize(QSize(16, 16));
    btn->setFixedSize(24, 24);
    btn->setToolTip(tooltip);
    return btn;
}

static void connectViewProjBtn(
        ButtonView3d* btn, WidgetOccView* view, V3d_TypeOfOrientation proj)
{
    QObject::connect(btn, &ButtonView3d::clicked, [=]{
        view->occV3dView()->SetProj(proj);
        view->fitAll();
    });
}

class PanelView3d : public QWidget {
public:
    PanelView3d(QWidget* parent = nullptr)
        : QWidget(parent)
    {}

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        const QRect frame = this->frameGeometry();
        const QRect surface(0, 0, frame.width(), frame.height());
        painter.fillRect(surface, this->palette().color(QPalette::Button));
    }
};

const int widgetMargin = 4;

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
    auto btnEditClipPlanes = Internal::createViewBtn(this, "clipping", tr("Edit clip planes"));
    btnEditClipPlanes->setCheckable(true);
    const int margin = Internal::widgetMargin;
    btnFitAll->move(margin, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewIso, btnFitAll, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBack, btnViewIso, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewFront, btnViewBack, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewLeft, btnViewFront, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewRight, btnViewLeft, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewTop, btnViewRight, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBottom, btnViewTop, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnEditClipPlanes, btnViewBottom, margin);

    Internal::connectViewProjBtn(btnViewIso, m_qtOccView, V3d_XposYnegZpos);
    Internal::connectViewProjBtn(btnViewBack, m_qtOccView, V3d_Xneg);
    Internal::connectViewProjBtn(btnViewFront, m_qtOccView, V3d_Xpos);
    Internal::connectViewProjBtn(btnViewLeft, m_qtOccView, V3d_Ypos);
    Internal::connectViewProjBtn(btnViewRight, m_qtOccView, V3d_Yneg);
    Internal::connectViewProjBtn(btnViewTop, m_qtOccView, V3d_Zpos);
    Internal::connectViewProjBtn(btnViewBottom, m_qtOccView, V3d_Zneg);
    QObject::connect(
                btnFitAll, &ButtonView3d::clicked,
                m_qtOccView, &WidgetOccView::fitAll);
    QObject::connect(
                btnEditClipPlanes, &ButtonView3d::clicked,
                this, &WidgetGuiDocumentView3d::toggleWidgetClipPlanes);

    m_firstBtnFrameRect = btnFitAll->frameGeometry();
}

GuiDocument *WidgetGuiDocumentView3d::guiDocument() const
{
    return m_guiDoc;
}

WidgetOccView *WidgetGuiDocumentView3d::widgetOccView() const
{
    return m_qtOccView;
}

void WidgetGuiDocumentView3d::toggleWidgetClipPlanes()
{
    if (m_widgetClipPlanes == nullptr) {
        auto panel = new Internal::PanelView3d(this);
        auto widget = new WidgetClipPlanes(this->widgetOccView(), panel);
        qtgui::QWidgetUtils::addContentsWidget(panel, widget);
        panel->show();
        panel->adjustSize();
        const int margin = Internal::widgetMargin;
        panel->move(margin, m_firstBtnFrameRect.bottom() + margin);
        m_widgetClipPlanes = widget;
        QObject::connect(
                    m_guiDoc, &GuiDocument::gpxBoundingBoxChanged,
                    widget, &WidgetClipPlanes::setRanges);
        widget->setRanges(m_guiDoc->gpxBoundingBox());
    }
    else {
        QWidget* panel = m_widgetClipPlanes->parentWidget();
        panel->setHidden(!panel->isHidden());
        m_widgetClipPlanes->setClippingOn(panel->isVisible());
    }
}

} // namespace Mayo
