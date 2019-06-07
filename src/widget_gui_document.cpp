/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_gui_document.h"

#include "button_flat.h"
#include "gpx_utils.h"
#include "gui_document.h"
#include "qt_occ_view_controller.h"
#include "theme.h"
#include "v3d_view_camera_animation.h"
#include "widget_occ_view.h"
#include "widget_clip_planes.h"

#include <fougtools/qttools/gui/qwidget_utils.h>
#include <QtGui/QPainter>
#include <QtWidgets/QBoxLayout>

namespace Mayo {

namespace Internal {

static ButtonFlat* createViewBtn(
        QWidget* parent, Theme::Icon icon, const QString& tooltip)
{
    auto btn = new ButtonFlat(parent);
    btn->setBackgroundBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Background));
    btn->setCheckedBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Checked));
    btn->setHoverBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Hover));
    btn->setIcon(mayoTheme()->icon(icon));
    btn->setIconSize(QSize(18, 18));
    btn->setFixedSize(24, 24);
    btn->setToolTip(tooltip);
    return btn;
}

class PanelView3d : public QWidget {
public:
    PanelView3d(QWidget* parent = nullptr)
        : QWidget(parent)
    {}

protected:
    void paintEvent(QPaintEvent*) override
    {
        WidgetGuiDocument::paintPanel(this);
    }
};

const int widgetMargin = 4;

} // namespace Internal

WidgetGuiDocument::WidgetGuiDocument(GuiDocument* guiDoc, QWidget* parent)
    : QWidget(parent),
      m_guiDoc(guiDoc),
      m_qtOccView(new WidgetOccView(guiDoc->v3dView(), this)),
      m_controller(new QtOccViewController(m_qtOccView)),
      m_cameraAnimation(new V3dViewCameraAnimation(guiDoc->v3dView(), this))
{
    m_cameraAnimation->setEasingCurve(QEasingCurve::OutExpo);
    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_qtOccView);
    this->setLayout(layout);

    auto btnFitAll = Internal::createViewBtn(this, Theme::Icon::Expand, tr("Fit All"));
    auto btnViewIso = Internal::createViewBtn(this, Theme::Icon::View3dIso, tr("Isometric"));
    auto btnViewBack = Internal::createViewBtn(this, Theme::Icon::View3dBack, tr("Back"));
    auto btnViewFront = Internal::createViewBtn(this, Theme::Icon::View3dFront, tr("Front"));
    auto btnViewLeft = Internal::createViewBtn(this, Theme::Icon::View3dLeft, tr("Left"));
    auto btnViewRight = Internal::createViewBtn(this, Theme::Icon::View3dRight, tr("Right"));
    auto btnViewTop = Internal::createViewBtn(this, Theme::Icon::View3dTop, tr("Top"));
    auto btnViewBottom = Internal::createViewBtn(this, Theme::Icon::View3dBottom, tr("Bottom"));
    auto btnEditClipping = Internal::createViewBtn(this, Theme::Icon::ClipPlane, tr("Edit clip planes"));
    btnEditClipping->setCheckable(true);
    const int margin = Internal::widgetMargin;
    btnFitAll->move(margin, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewIso, btnFitAll, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBack, btnViewIso, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewFront, btnViewBack, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewLeft, btnViewFront, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewRight, btnViewLeft, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewTop, btnViewRight, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnViewBottom, btnViewTop, margin);
    qtgui::QWidgetUtils::moveWidgetRightTo(btnEditClipping, btnViewBottom, margin);

    this->connectViewProjButton(btnViewIso, V3d_XposYnegZpos);
    this->connectViewProjButton(btnViewIso, V3d_XposYnegZpos);
    this->connectViewProjButton(btnViewBack, V3d_Xneg);
    this->connectViewProjButton(btnViewFront, V3d_Xpos);
    this->connectViewProjButton(btnViewLeft, V3d_Ypos);
    this->connectViewProjButton(btnViewRight, V3d_Yneg);
    this->connectViewProjButton(btnViewTop, V3d_Zpos);
    this->connectViewProjButton(btnViewBottom, V3d_Zneg);
    QObject::connect(btnFitAll, &ButtonFlat::clicked, [=]{
        m_cameraAnimation->configure(&GpxUtils::V3dView_fitAll);
        m_cameraAnimation->start(QAbstractAnimation::KeepWhenStopped);
    });
    QObject::connect(
                m_controller, &BaseV3dViewController::dynamicActionStarted,
                m_cameraAnimation, &V3dViewCameraAnimation::stop);
    QObject::connect(
                m_controller, &BaseV3dViewController::viewScaled,
                m_cameraAnimation, &V3dViewCameraAnimation::stop);
    QObject::connect(
                btnEditClipping, &ButtonFlat::clicked,
                this, &WidgetGuiDocument::toggleWidgetClipPlanes);

    const QRect rectFirstBtn = btnFitAll->frameGeometry();
    const QRect rectLastBtn = btnEditClipping->frameGeometry();
    m_rectControls.setCoords(
                rectFirstBtn.left(), rectFirstBtn.top(),
                rectLastBtn.right(), rectLastBtn.bottom());
}

GuiDocument* WidgetGuiDocument::guiDocument() const
{
    return m_guiDoc;
}

BaseV3dViewController* WidgetGuiDocument::controller() const
{
    return m_controller;
}

QRect WidgetGuiDocument::rectControls() const
{
    return m_rectControls;
}

void WidgetGuiDocument::paintPanel(QWidget* widget)
{
    QPainter painter(widget);
    const QRect frame = widget->frameGeometry();
    const QRect surface(0, 0, frame.width(), frame.height());
    const QColor panelColor = mayoTheme()->color(Theme::Color::Palette_Window);
    painter.fillRect(surface, panelColor);
}

void WidgetGuiDocument::connectViewProjButton(ButtonFlat* btn, V3d_TypeOfOrientation proj)
{
    QObject::connect(btn, &ButtonFlat::clicked, [=]{
        m_cameraAnimation->configure([=](Handle_V3d_View view) {
            view->SetProj(proj);
            GpxUtils::V3dView_fitAll(view);
        });
        m_cameraAnimation->start(QAbstractAnimation::KeepWhenStopped);
    });
}

void WidgetGuiDocument::toggleWidgetClipPlanes()
{
    if (!m_widgetClipPlanes) {
        auto panel = new Internal::PanelView3d(this);
        auto widget = new WidgetClipPlanes(m_guiDoc->v3dView(), panel);
        qtgui::QWidgetUtils::addContentsWidget(panel, widget);
        panel->show();
        panel->adjustSize();
        const int margin = Internal::widgetMargin;
        panel->move(margin, m_rectControls.bottom() + margin);
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
