/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_gui_document.h"

#include "../graphics/graphics_utils.h"
#include "../graphics/v3d_view_camera_animation.h"
#include "../gui/gui_document.h"
#include "button_flat.h"
#include "theme.h"
#include "widget_clip_planes.h"
#include "widget_occ_view.h"
#include "widget_occ_view_controller.h"
#include "widgets_utils.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QWidgetAction>

namespace Mayo {

namespace Internal {

static ButtonFlat* createViewBtn(QWidget* parent, Theme::Icon icon, const QString& tooltip)
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
    void paintEvent(QPaintEvent*) override {
        WidgetGuiDocument::paintPanel(this);
    }
};

class MenuIconSizeStyle : public QProxyStyle {
public:
    void setMenuIconSize(int size) {
        m_menuIconSize = size;
    }

    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const override
    {
        if (metric == QStyle::PM_SmallIconSize && m_menuIconSize > 0)
            return m_menuIconSize;

        return QProxyStyle::pixelMetric(metric, option, widget);
    }

private:
    int m_menuIconSize = -1;
};

const int widgetMargin = 4;

} // namespace Internal

WidgetGuiDocument::WidgetGuiDocument(GuiDocument* guiDoc, QWidget* parent)
    : QWidget(parent),
      m_guiDoc(guiDoc),
      m_qtOccView(new WidgetOccView(guiDoc->v3dView(), this)),
      m_controller(new WidgetOccViewController(m_qtOccView))
{
    {
        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_qtOccView);
        this->setLayout(layout);
    }

    m_btnFitAll = Internal::createViewBtn(this, Theme::Icon::Expand, tr("Fit All"));
    m_btnEditClipping = Internal::createViewBtn(this, Theme::Icon::ClipPlane, tr("Edit clip planes"));
    m_btnEditClipping->setCheckable(true);

    QObject::connect(m_btnFitAll, &ButtonFlat::clicked, this, [=]{
        m_guiDoc->runViewCameraAnimation(&GraphicsUtils::V3dView_fitAll);
    });
    QObject::connect(
                m_btnEditClipping, &ButtonFlat::clicked,
                this, &WidgetGuiDocument::toggleWidgetClipPlanes);
    QObject::connect(
                m_controller, &V3dViewController::dynamicActionStarted,
                m_guiDoc, &GuiDocument::stopViewCameraAnimation);
    QObject::connect(
                m_controller, &V3dViewController::viewScaled,
                m_guiDoc, &GuiDocument::stopViewCameraAnimation);
    QObject::connect(
                m_controller, &V3dViewController::mouseClicked, this, [=](Qt::MouseButton btn) {
        if (btn == Qt::MouseButton::LeftButton) {
            if (!m_guiDoc->processAction(m_guiDoc->graphicsScene()->currentHighlightedOwner()))
                m_guiDoc->graphicsScene()->select();
        }
    });
    QObject::connect(m_controller, &WidgetOccViewController::multiSelectionToggled, this, [=](bool on) {
        m_guiDoc->graphicsScene()->setSelectionMode(
                    on ? GraphicsScene::SelectionMode::Multi : GraphicsScene::SelectionMode::Single);
    });
    QObject::connect(
                m_guiDoc, &GuiDocument::viewTrihedronModeChanged,
                this, &WidgetGuiDocument::recreateViewControls);

    this->recreateViewControls();
}

void WidgetGuiDocument::paintPanel(QWidget* widget)
{
    QPainter painter(widget);
    const QRect frame = widget->frameGeometry();
    const QRect surface(0, 0, frame.width(), frame.height());
    const QColor panelColor = mayoTheme()->color(Theme::Color::Palette_Window);
    painter.fillRect(surface, panelColor);
}

void WidgetGuiDocument::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    this->layoutViewControls();
    this->layoutWidgetClipPlanes();
}

void WidgetGuiDocument::toggleWidgetClipPlanes()
{
    if (!m_widgetClipPlanes) {
        auto panel = new Internal::PanelView3d(this);
        auto widget = new WidgetClipPlanes(m_guiDoc->v3dView(), panel);
        WidgetsUtils::addContentsWidget(panel, widget);
        panel->show();
        panel->adjustSize();
        m_widgetClipPlanes = widget;
        QObject::connect(
                    m_guiDoc, &GuiDocument::graphicsBoundingBoxChanged,
                    widget, &WidgetClipPlanes::setRanges);
        widget->setRanges(m_guiDoc->graphicsBoundingBox());
    }
    else {
        QWidget* panel = m_widgetClipPlanes->parentWidget();
        panel->setHidden(!panel->isHidden());
        m_widgetClipPlanes->setClippingOn(panel->isVisible());
    }

    this->layoutWidgetClipPlanes();
}

void WidgetGuiDocument::layoutWidgetClipPlanes()
{
    auto fnPanelPos = [=](QWidget* panel) -> QPoint {
        const int margin = Internal::widgetMargin;
        if (m_guiDoc->viewTrihedronMode() != GuiDocument::ViewTrihedronMode::AisViewCube)
            return QPoint(margin, this->viewControlsRect().bottom() + margin);

        switch (m_guiDoc->viewTrihedronCorner()) {
        case Qt::TopLeftCorner:
            return QPoint(margin, this->viewControlsRect().bottom() + margin);
        case Qt::TopRightCorner:
            return QPoint(this->width() - panel->width(),
                          this->viewControlsRect().bottom() + margin);
        case Qt::BottomLeftCorner:
            return QPoint(margin, this->viewControlsRect().top() - panel->height() - margin);
        case Qt::BottomRightCorner:
            return QPoint(this->width() - panel->width(),
                          this->viewControlsRect().top() - panel->height() - margin);
        } // endswitch

        return QPoint(margin, this->viewControlsRect().bottom() + margin);
    };

    QWidget* panel = m_widgetClipPlanes ? m_widgetClipPlanes->parentWidget() : nullptr;
    if (panel && panel->isVisible())
        panel->move(fnPanelPos(panel));
}

void WidgetGuiDocument::recreateViewControls()
{
    for (QWidget* widget : m_vecWidgetForViewProj)
        delete widget;

    m_vecWidgetForViewProj.clear();

    struct ButtonCreationData {
        Theme::Icon icon;
        QString text;
        V3d_TypeOfOrientation proj;
    };
    const ButtonCreationData btnCreationData[] = {
        { Theme::Icon::View3dIso, tr("Isometric"), V3d_XposYnegZpos },
        { Theme::Icon::View3dBack, tr("Back"), V3d_Ypos },
        { Theme::Icon::View3dFront, tr("Front"), V3d_Yneg },
        { Theme::Icon::View3dLeft, tr("Left"), V3d_Xneg },
        { Theme::Icon::View3dRight, tr("Right"), V3d_Xpos },
        { Theme::Icon::View3dTop, tr("Top"), V3d_Zpos },
        { Theme::Icon::View3dBottom, tr("Bottom"), V3d_Zneg }
    };
    if (m_guiDoc->viewTrihedronMode() == GuiDocument::ViewTrihedronMode::AisViewCube) {
        static Internal::MenuIconSizeStyle* menuStyle = nullptr;
        if (!menuStyle) {
            menuStyle = new Internal::MenuIconSizeStyle;
            menuStyle->setMenuIconSize(m_btnFitAll->iconSize().width());
        }

        const QString strTemplateTooltip =
                tr("<b>Left-click</b>: popup menu of pre-defined views\n"
                   "<b>CTRL+Left-click</b>: apply '%1' view");
        auto btnViewMenu = Internal::createViewBtn(this, Theme::Icon::View3dIso, QString());
        btnViewMenu->setToolTip(strTemplateTooltip.arg(btnCreationData[0].text));
        btnViewMenu->setData(static_cast<int>(btnCreationData[0].proj));
        auto menuBtnView = new QMenu(btnViewMenu);
        menuBtnView->setStyle(menuStyle);
        m_vecWidgetForViewProj.push_back(btnViewMenu);
        for (const ButtonCreationData& btnData : btnCreationData) {
            auto action = menuBtnView->addAction(mayoTheme()->icon(btnData.icon), btnData.text);
            QObject::connect(action, &QAction::triggered, this, [=]{
                m_guiDoc->setViewCameraOrientation(btnData.proj);
                btnViewMenu->setIcon(action->icon());
                btnViewMenu->setToolTip(strTemplateTooltip.arg(btnData.text));
                btnViewMenu->setData(static_cast<int>(btnData.proj));
                btnViewMenu->update();
            });
        }

        //QStyle::PE_IndicatorArrowDown
        QObject::connect(btnViewMenu, &ButtonFlat::clicked, this, [=]{
            const Qt::KeyboardModifiers keyMods = QGuiApplication::queryKeyboardModifiers();
            if (!keyMods.testFlag(Qt::ControlModifier)) {
                menuBtnView->popup(btnViewMenu->mapToGlobal({ 0, btnViewMenu->height() }));
            }
            else {
                m_guiDoc->setViewCameraOrientation(
                            static_cast<V3d_TypeOfOrientation>(btnViewMenu->data().toInt()));
            }
        });
    }
    else {
        for (const ButtonCreationData& btnData : btnCreationData) {
            auto btnViewProj = Internal::createViewBtn(this, btnData.icon, btnData.text);
            QObject::connect(btnViewProj, &ButtonFlat::clicked, this, [=]{
                m_guiDoc->setViewCameraOrientation(btnData.proj);
            });
            m_vecWidgetForViewProj.push_back(btnViewProj);
        }
    }

    this->layoutViewControls();
}

QRect WidgetGuiDocument::viewControlsRect() const
{
    const QRect rectFirstBtn = m_btnFitAll->frameGeometry();
    const QRect rectLastBtn = m_btnEditClipping->frameGeometry();
    QRect rect;
    rect.setCoords(
                rectFirstBtn.left(), rectFirstBtn.top(),
                rectLastBtn.right(), rectLastBtn.bottom());
    return rect;
}

void WidgetGuiDocument::layoutViewControls()
{
    const int margin = Internal::widgetMargin;
    auto fnGetViewControlsPos = [=]() -> QPoint {
        if (m_guiDoc->viewTrihedronMode() == GuiDocument::ViewTrihedronMode::AisViewCube) {
            const int btnSize = m_btnFitAll->width();
            const int viewCubeBndSize = m_guiDoc->aisViewCubeBoundingSize();
            const int ctrlCount = 2 + m_vecWidgetForViewProj.size();
            const int ctrlWidth = ctrlCount * btnSize + (ctrlCount - 1) * margin;
            const int ctrlHeight = btnSize;
            const int ctrlXOffset = (viewCubeBndSize - ctrlWidth) / 2;
            switch (m_guiDoc->viewTrihedronCorner()) {
            case Qt::TopLeftCorner:
                return { ctrlXOffset, viewCubeBndSize + margin };
            case Qt::TopRightCorner:
                return { this->width() - viewCubeBndSize + ctrlXOffset, viewCubeBndSize + margin };
            case Qt::BottomLeftCorner:
                return { ctrlXOffset, this->height() - viewCubeBndSize - margin - ctrlHeight };
            case Qt::BottomRightCorner:
                return { this->width() - viewCubeBndSize + ctrlXOffset,
                         this->height() - viewCubeBndSize - margin - ctrlHeight };
            } // endswitch
        }

        return { margin, margin };
    };

    m_btnFitAll->move(fnGetViewControlsPos());
    const QWidget* widgetLast = m_btnFitAll;
    for (QWidget* widget : m_vecWidgetForViewProj) {
        WidgetsUtils::moveWidgetRightTo(widget, widgetLast, margin);
        widgetLast = widget;
    }

    WidgetsUtils::moveWidgetRightTo(m_btnEditClipping, widgetLast, margin);
}

} // namespace Mayo
