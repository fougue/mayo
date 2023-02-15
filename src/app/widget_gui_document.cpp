/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_gui_document.h"

#include "../base/cpp_utils.h"
#include "../base/unit_system.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"
#include "../gui/v3d_view_camera_animation.h"
#include "button_flat.h"
#include "theme.h"
#include "widget_clip_planes.h"
#include "widget_explode_assembly.h"
#include "widget_measure.h"
#include "widget_occ_view.h"
#include "widget_occ_view_controller.h"
#include "qtwidgets_utils.h"

#include <QtCore/QtDebug>
#include <QtCore/QAbstractAnimation>
#include <QtCore/QEasingCurve>
#include <QtGui/QPainter>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QWidgetAction>
#include <Standard_Version.hxx>

namespace Mayo {

namespace {

// Provides implementation of IAnimationBackend based on QAbstractAnimation
class QtAnimationBackend : public IAnimationBackend {
public:
    QtAnimationBackend(QEasingCurve::Type easingType = QEasingCurve::Linear)
        : m_easingCurve(easingType)
    {
    }

    void setDuration(QuantityTime t) override {
        m_impl.m_duration_ms = UnitSystem::milliseconds(t);
    }

    bool isRunning() const override {
        return m_impl.state() == QAbstractAnimation::Running;
    }

    void start() override {
        m_impl.start(QAbstractAnimation::KeepWhenStopped);
    }

    void stop() override {
        m_impl.stop();
    }

    double valueForProgress(double p) const override {
        return m_easingCurve.valueForProgress(p);
    }

    void setTimerCallback(std::function<void(QuantityTime)> fn) override {
        m_impl.m_callback = std::move(fn);
    }

private:
    class AnimationImpl : public QAbstractAnimation {
    public:
        double m_duration_ms = 1000.;
        std::function<void(QuantityTime)> m_callback;

        int duration() const override {
            return static_cast<int>(m_duration_ms);
        }

    protected:
        void updateCurrentTime(int currentTime) override {
            m_callback(currentTime * Quantity_Millisecond);
        }
    };

    AnimationImpl m_impl;
    QEasingCurve m_easingCurve;
};

// Provides an overlay widget to be used within 3D view
class PanelView3d : public QWidget {
public:
    PanelView3d(WidgetGuiDocument* parent = nullptr)
        : QWidget(parent)
    {}

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        const QRect frame = this->frameGeometry();
        const QRect surface(0, 0, frame.width(), frame.height());
        auto widgetGuiDocument = static_cast<const WidgetGuiDocument*>(this->parentWidget());
        painter.fillRect(surface, widgetGuiDocument->panelBackgroundColor());
    }    
};

// Provides style to redefine icon size of menu items
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

// Default margin to be used in widgets
const int Internal_widgetMargin = 4;

} // namespace

WidgetGuiDocument::WidgetGuiDocument(GuiDocument* guiDoc, QWidget* parent)
    : QWidget(parent),
      m_guiDoc(guiDoc),
      m_qtOccView(IWidgetOccView::create(guiDoc->v3dView(), this)),
      m_controller(new WidgetOccViewController(m_qtOccView))
{
    {
        auto layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_qtOccView->widget());
    }

    auto widgetBtnsContents = new QWidget;
    auto layoutBtns = new QHBoxLayout(widgetBtnsContents);
    layoutBtns->setSpacing(Internal_widgetMargin + 2);
    layoutBtns->setContentsMargins(2, 2, 2, 2);
    m_btnFitAll = this->createViewBtn(widgetBtnsContents, Theme::Icon::Expand, tr("Fit All"));
    m_btnEditClipping = this->createViewBtn(widgetBtnsContents, Theme::Icon::ClipPlane, tr("Edit clip planes"));
    m_btnEditClipping->setCheckable(true);
    m_btnExplode = this->createViewBtn(widgetBtnsContents, Theme::Icon::Multiple, tr("Explode assemblies"));
    m_btnExplode->setCheckable(true);
    m_btnMeasure = this->createViewBtn(widgetBtnsContents, Theme::Icon::Measure, tr("Measure shapes"));
    m_btnMeasure->setCheckable(true);

    layoutBtns->addWidget(m_btnFitAll);
    this->recreateMenuViewProjections(widgetBtnsContents);
    layoutBtns->addWidget(m_btnEditClipping);
    layoutBtns->addWidget(m_btnExplode);
    layoutBtns->addWidget(m_btnMeasure);
    m_widgetBtns = this->createWidgetPanelContainer(widgetBtnsContents);

    auto gfxScene = m_guiDoc->graphicsScene();
    QObject::connect(m_btnFitAll, &ButtonFlat::clicked, this, [=]{
        m_guiDoc->runViewCameraAnimation(&GraphicsUtils::V3dView_fitAll);
    });
    QObject::connect(
                m_btnEditClipping, &ButtonFlat::checked,
                this, &WidgetGuiDocument::toggleWidgetClipPlanes
    );
    QObject::connect(
                m_btnExplode, &ButtonFlat::checked,
                this, &WidgetGuiDocument::toggleWidgetExplode
    );
    QObject::connect(
                m_btnMeasure, &ButtonFlat::checked,
                this, &WidgetGuiDocument::toggleWidgetMeasure
    );
    m_controller->signalDynamicActionStarted.connectSlot([=]{ m_guiDoc->stopViewCameraAnimation(); });
    m_controller->signalViewScaled.connectSlot([=]{ m_guiDoc->stopViewCameraAnimation(); });
    m_controller->signalMouseButtonClicked.connectSlot([=](Aspect_VKeyMouse btn) {
        if (btn == Aspect_VKeyMouse_LeftButton && !m_guiDoc->processAction(gfxScene->currentHighlightedOwner())) {
            gfxScene->select();
            m_qtOccView->redraw();
        }
    });
    m_controller->signalMultiSelectionToggled.connectSlot([=](bool on) {
        auto mode = on ? GraphicsScene::SelectionMode::Multi : GraphicsScene::SelectionMode::Single;
        gfxScene->setSelectionMode(mode);
    });

    m_guiDoc->viewCameraAnimation()->setBackend(std::make_unique<QtAnimationBackend>(QEasingCurve::OutExpo));
    m_guiDoc->viewCameraAnimation()->setRenderFunction([=](const Handle_V3d_View& view){
        if (view == m_qtOccView->v3dView())
            m_qtOccView->redraw();
    });
}

Document::Identifier WidgetGuiDocument::documentIdentifier() const
{
    return m_guiDoc->document()->identifier();
}

QColor WidgetGuiDocument::panelBackgroundColor() const
{
    QColor color = mayoTheme()->color(Theme::Color::Palette_Window);
    if (m_qtOccView->supportsWidgetOpacity())
        color.setAlpha(175);

    return color;
}

void WidgetGuiDocument::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    this->layoutViewControls();
    this->layoutWidgetPanel(m_widgetClipPlanes);
    this->layoutWidgetPanel(m_widgetExplodeAsm);
    this->layoutWidgetPanel(m_widgetMeasure);
}

QWidget* WidgetGuiDocument::createWidgetPanelContainer(QWidget* widgetContents)
{
    auto panel = new PanelView3d(this);
    QtWidgetsUtils::addContentsWidget(panel, widgetContents);
    panel->show();
    panel->adjustSize();
    return panel;
}

void WidgetGuiDocument::updageWidgetPanelControls(QWidget* panelWidget, ButtonFlat* btnPanel)
{
    this->exclusiveButtonCheck(btnPanel);
    if (panelWidget) {
        panelWidget->parentWidget()->setVisible(btnPanel->isChecked());
        this->layoutWidgetPanel(panelWidget);
    }
}

void WidgetGuiDocument::toggleWidgetClipPlanes(bool on)
{
    if (m_widgetClipPlanes) {
        m_widgetClipPlanes->setClippingOn(on);
    }
    else if (on) {
        m_widgetClipPlanes = new WidgetClipPlanes(m_guiDoc->v3dView());
        this->createWidgetPanelContainer(m_widgetClipPlanes);
        m_guiDoc->signalGraphicsBoundingBoxChanged.connectSlot(&WidgetClipPlanes::setRanges, m_widgetClipPlanes);
        m_widgetClipPlanes->setRanges(m_guiDoc->graphicsBoundingBox());
    }

    this->updageWidgetPanelControls(m_widgetClipPlanes, m_btnEditClipping);
}

void WidgetGuiDocument::toggleWidgetExplode(bool on)
{
    if (!m_widgetExplodeAsm && on) {
        m_widgetExplodeAsm = new WidgetExplodeAssembly(m_guiDoc);
        this->createWidgetPanelContainer(m_widgetExplodeAsm);
    }

    this->updageWidgetPanelControls(m_widgetExplodeAsm, m_btnExplode);
}

void WidgetGuiDocument::toggleWidgetMeasure(bool on)
{
    if (!m_widgetMeasure && on) {
        m_widgetMeasure = new WidgetMeasure(m_guiDoc);
        auto container = this->createWidgetPanelContainer(m_widgetMeasure);
        QObject::connect(
                    m_widgetMeasure, &WidgetMeasure::sizeAdjustmentRequested,
                    container, &QWidget::adjustSize,
                    Qt::QueuedConnection
        );
    }

    if (m_widgetMeasure)
        m_widgetMeasure->setMeasureOn(on);

    this->updageWidgetPanelControls(m_widgetMeasure, m_btnMeasure);
}

void WidgetGuiDocument::exclusiveButtonCheck(ButtonFlat* btnCheck)
{
    if (!btnCheck || !btnCheck->isChecked())
        return;

    ButtonFlat* arrayToggleBtn[] = { m_btnEditClipping, m_btnExplode, m_btnMeasure };
    for (ButtonFlat* btn : arrayToggleBtn) {
        assert(btn->isCheckable());
        if (btn != btnCheck)
            btn->setChecked(false);
    }
}

void WidgetGuiDocument::layoutWidgetPanel(QWidget* panel)
{
    auto fnPanelPos = [=](QWidget* panel) -> QPoint {
        const QRect ctrlRect = this->viewControlsRect();
        const int margin = Internal_widgetMargin;
        if (m_guiDoc->viewTrihedronMode() != GuiDocument::ViewTrihedronMode::AisViewCube)
            return QPoint(margin, ctrlRect.bottom() + margin);

        switch (m_guiDoc->viewTrihedronCorner()) {
        case Aspect_TOTP_LEFT_UPPER:
            return QPoint(ctrlRect.left(), ctrlRect.bottom() + margin);
        case Aspect_TOTP_RIGHT_UPPER:
            return QPoint(this->width() - panel->width(), ctrlRect.bottom() + margin);
        case Aspect_TOTP_LEFT_LOWER:
            return QPoint(margin, ctrlRect.top() - panel->height() - margin);
        case Aspect_TOTP_RIGHT_LOWER:
            return QPoint(this->width() - panel->width(), ctrlRect.top() - panel->height() - margin);
        default:
            return QPoint(margin, ctrlRect.bottom() + margin);
        } // endswitch
    };

    QWidget* widget = panel ? panel->parentWidget() : nullptr;
    if (widget && widget->isVisible())
        widget->move(fnPanelPos(widget));
}

ButtonFlat* WidgetGuiDocument::createViewBtn(QWidget* parent, Theme::Icon icon, const QString& tooltip) const
{
    const QColor bkgndColor =
            m_qtOccView->supportsWidgetOpacity() ?
                Qt::transparent :
                mayoTheme()->color(Theme::Color::ButtonView3d_Background);

    auto btn = new ButtonFlat(parent);
    btn->setBackgroundBrush(bkgndColor);
    btn->setCheckedBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Checked));
    btn->setHoverBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Hover));
    btn->setIcon(mayoTheme()->icon(icon));
    btn->setIconSize(QSize(20, 20));
    btn->setFixedSize(28, 28);
    btn->setToolTip(tooltip);
    return btn;
}

void WidgetGuiDocument::recreateMenuViewProjections(QWidget* container)
{
    for (QWidget* widget : m_vecWidgetForViewProj)
        delete widget;

    m_vecWidgetForViewProj.clear();

    struct ButtonCreationData {
        V3d_TypeOfOrientation proj;
        Theme::Icon icon;
        QString text;
    };
    const ButtonCreationData btnCreationData[] = {
        { V3d_XposYnegZpos, Theme::Icon::View3dIso, tr("Isometric") },
        { V3d_Ypos, Theme::Icon::View3dBack,   tr("Back")},
        { V3d_Yneg, Theme::Icon::View3dFront,  tr("Front") },
        { V3d_Xneg, Theme::Icon::View3dLeft,   tr("Left") },
        { V3d_Xpos, Theme::Icon::View3dRight,  tr("Right") },
        { V3d_Zpos, Theme::Icon::View3dTop,    tr("Top") },
        { V3d_Zneg, Theme::Icon::View3dBottom, tr("Bottom") }
    };
    if (m_guiDoc->viewTrihedronMode() == GuiDocument::ViewTrihedronMode::AisViewCube) {
        static MenuIconSizeStyle* menuStyle = nullptr;
        if (!menuStyle) {
            menuStyle = new MenuIconSizeStyle;
            menuStyle->setMenuIconSize(m_btnFitAll->iconSize().width());
        }

        const QString strTemplateTooltip =
                tr("<b>Left-click</b>: popup menu of pre-defined views\n"
                   "<b>CTRL+Left-click</b>: apply '%1' view");
        auto btnViewMenu = this->createViewBtn(container, Theme::Icon::View3dIso, QString());
        container->layout()->addWidget(btnViewMenu);
        btnViewMenu->setToolTip(strTemplateTooltip.arg(btnCreationData[0].text));
        btnViewMenu->setData(static_cast<int>(btnCreationData[0].proj));
        auto menuBtnView = new QMenu(btnViewMenu);
        menuBtnView->setStyle(menuStyle);
        const QString strPanelBkgndColor = this->panelBackgroundColor().name(QColor::HexArgb);
        menuBtnView->setStyleSheet(QString("QMenu { background:%1; border: 0px }").arg(strPanelBkgndColor));
        menuBtnView->setWindowFlags(menuBtnView->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        if (m_qtOccView->supportsWidgetOpacity())
            menuBtnView->setAttribute(Qt::WA_TranslucentBackground);

        m_vecWidgetForViewProj.push_back(btnViewMenu);
        for (const ButtonCreationData& btnData : btnCreationData) {
            auto action = menuBtnView->addAction(mayoTheme()->icon(btnData.icon), btnData.text);
            QObject::connect(action, &QAction::triggered, this, [=]{
                m_guiDoc->setViewCameraOrientation(btnData.proj);
                btnViewMenu->setIcon(action->icon());
                btnViewMenu->setToolTip(strTemplateTooltip.arg(btnData.text));
                btnViewMenu->setData(int(btnData.proj));
                btnViewMenu->update();
            });
        }

        //QStyle::PE_IndicatorArrowDown
        QObject::connect(btnViewMenu, &ButtonFlat::clicked, this, [=]{
            const Qt::KeyboardModifiers keyMods = QGuiApplication::queryKeyboardModifiers();
            if (!keyMods.testFlag(Qt::ControlModifier))
                menuBtnView->popup(btnViewMenu->mapToGlobal(QPoint{ 0, container->height() }));
            else
                m_guiDoc->setViewCameraOrientation(V3d_TypeOfOrientation(btnViewMenu->data().toInt()));
        });
    }
    else {
        for (const ButtonCreationData& btnData : btnCreationData) {
            auto btnViewProj = this->createViewBtn(container, btnData.icon, btnData.text);
            container->layout()->addWidget(btnViewProj);
            QObject::connect(btnViewProj, &ButtonFlat::clicked, this, [=]{
                m_guiDoc->setViewCameraOrientation(btnData.proj);
            });
            m_vecWidgetForViewProj.push_back(btnViewProj);
        }
    }
}

QRect WidgetGuiDocument::viewControlsRect() const
{
    return m_widgetBtns->frameGeometry();
}

void WidgetGuiDocument::layoutViewControls()
{
    const int margin = Internal_widgetMargin + 2;
    auto fnGetViewControlsPos = [=]() -> QPoint {
        if (m_guiDoc->viewTrihedronMode() == GuiDocument::ViewTrihedronMode::AisViewCube) {
            const int btnSize = m_btnFitAll->width();
            const int viewCubeBndSize = m_guiDoc->aisViewCubeBoundingSize() / m_guiDoc->devicePixelRatio();
            const int ctrlHeight = btnSize;
            const int ctrlXOffset = margin;
            switch (m_guiDoc->viewTrihedronCorner()) {
            case Aspect_TOTP_LEFT_UPPER:
                return { ctrlXOffset, viewCubeBndSize + margin };
            case Aspect_TOTP_RIGHT_UPPER:
                return { this->width() - viewCubeBndSize + ctrlXOffset, viewCubeBndSize + margin };
            case Aspect_TOTP_LEFT_LOWER:
                return { ctrlXOffset, this->height() - viewCubeBndSize - margin - ctrlHeight };
            case Aspect_TOTP_RIGHT_LOWER:
                return { this->width() - viewCubeBndSize + ctrlXOffset,
                         this->height() - viewCubeBndSize - margin - ctrlHeight };
            default:
                return { margin, margin };
            } // endswitch
        }

        return { margin, margin };
    };

    m_widgetBtns->move(fnGetViewControlsPos());
}

} // namespace Mayo
