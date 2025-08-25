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
#include "../qtbackend/qt_animation_backend.h"
#include "button_flat.h"
#include "theme.h"
#include "widget_clip_planes.h"
#include "widget_explode_assembly.h"
#include "widget_grid.h"
#include "widget_measure.h"
#include "widget_occ_view.h"
#include "widget_occ_view_controller.h"
#include "qtwidgets_utils.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QWidgetAction>
#include <Standard_Version.hxx>

namespace Mayo {

namespace {

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
    layoutBtns->setSpacing(0);
    layoutBtns->setContentsMargins(QMargins{0, 0, 0, 0});

    m_btnFitAll = this->createViewBtn(widgetBtnsContents, Theme::Icon::Expand, tr("Fit All"));

    m_btnGrid = this->createViewBtn(widgetBtnsContents, Theme::Icon::Grid, tr("Edit Grid"));
    m_btnGrid->setCheckable(true);

    m_btnEditClipping = this->createViewBtn(widgetBtnsContents, Theme::Icon::ClipPlane, tr("Edit clip planes"));
    m_btnEditClipping->setCheckable(true);

    m_btnExplode = this->createViewBtn(widgetBtnsContents, Theme::Icon::Multiple, tr("Explode assemblies"));
    m_btnExplode->setCheckable(true);

    m_btnMeasure = this->createViewBtn(widgetBtnsContents, Theme::Icon::Measure, tr("Measure shapes"));
    m_btnMeasure->setCheckable(true);

    layoutBtns->addWidget(m_btnFitAll);
    this->recreateMenuViewProjections(widgetBtnsContents);
    layoutBtns->addWidget(m_btnGrid);
    layoutBtns->addWidget(m_btnEditClipping);
    layoutBtns->addWidget(m_btnExplode);
    layoutBtns->addWidget(m_btnMeasure);
    m_widgetBtns = this->createWidgetPanelContainer(widgetBtnsContents);

    auto gfxScene = m_guiDoc->graphicsScene();
    gfxScene->signalRedrawRequested.connectSlot([=](const OccHandle<V3d_View>& view) {
        if (view == m_qtOccView->v3dView())
            m_qtOccView->redraw();
    });
    QObject::connect(m_btnFitAll, &ButtonFlat::clicked, this, [=]{
        m_guiDoc->runViewCameraAnimation([=](OccHandle<V3d_View> view) {
            auto bndBoxFlags = GuiDocument::OnlySelectedGraphics | GuiDocument::OnlyVisibleGraphics;
            GraphicsUtils::V3dView_fitAll(view, this->guiDocument()->graphicsBoundingBox(bndBoxFlags));
        });
    });
    QObject::connect(m_btnGrid, &ButtonFlat::checked, this, &WidgetGuiDocument::toggleWidgetGrid);
    QObject::connect(m_btnEditClipping, &ButtonFlat::checked, this, &WidgetGuiDocument::toggleWidgetClipPlanes);
    QObject::connect(m_btnExplode, &ButtonFlat::checked, this, &WidgetGuiDocument::toggleWidgetExplode);
    QObject::connect(m_btnMeasure, &ButtonFlat::checked, this, &WidgetGuiDocument::toggleWidgetMeasure);
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
    m_guiDoc->viewCameraAnimation()->setRenderFunction([=](const OccHandle<V3d_View>& view){
        if (view == m_qtOccView->v3dView())
            m_qtOccView->redraw();
    });
    m_guiDoc->signalViewTrihedronCornerChanged.connectSlot([=](Aspect_TypeOfTriedronPosition) {
        this->layoutViewControls();
        this->layoutWidgetPanels();
        m_guiDoc->graphicsView().redraw();
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
    this->layoutWidgetPanels();
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

void adjustWidgetSize(QWidget* widget)
{
    widget->updateGeometry();
    if (static_cast<const PanelView3d*>(widget->parentWidget()))
        widget->parentWidget()->adjustSize();
}

void WidgetGuiDocument::toggleWidgetGrid(bool on)
{
    if (!m_widgetGrid && on) {
        m_widgetGrid = new WidgetGrid(m_guiDoc->graphicsView());
        auto container = this->createWidgetPanelContainer(m_widgetGrid);
        QObject::connect(
            m_widgetGrid, &WidgetGrid::sizeAdjustmentRequested,
            container, [=]{ adjustWidgetSize(m_widgetGrid); },
            Qt::QueuedConnection
        );
    }

    this->updageWidgetPanelControls(m_widgetGrid, m_btnGrid);
}

void WidgetGuiDocument::toggleWidgetClipPlanes(bool on)
{
    if (!m_widgetClipPlanes && on) {
        m_widgetClipPlanes = new WidgetClipPlanes(m_guiDoc->graphicsView());
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
            container, [=]{ adjustWidgetSize(m_widgetMeasure); },
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

    ButtonFlat* arrayToggleBtn[] = { m_btnGrid, m_btnEditClipping, m_btnExplode, m_btnMeasure };
    for (ButtonFlat* btn : arrayToggleBtn) {
        assert(btn->isCheckable());
        if (btn != btnCheck)
            btn->setChecked(false);
    }
}

void WidgetGuiDocument::layoutWidgetPanel(QWidget* panel)
{
    QWidget* widget = panel ? panel->parentWidget() : nullptr;
    if (widget) {
        const QRect ctrlRect = this->viewControlsRect();
        const int margin = panel->devicePixelRatio() * Internal_widgetMargin;
        widget->move(ctrlRect.left(), ctrlRect.bottom() + margin);
    }
}

void WidgetGuiDocument::layoutWidgetPanels()
{
    QWidget* widgetPanels[] = {
        m_widgetGrid, m_widgetClipPlanes, m_widgetExplodeAsm, m_widgetMeasure
    };
    for (QWidget* panel : widgetPanels)
        this->layoutWidgetPanel(panel);
}

ButtonFlat* WidgetGuiDocument::createViewBtn(QWidget* parent, Theme::Icon icon, const QString& tooltip) const
{
    const QColor bkgndColor =
        m_qtOccView->supportsWidgetOpacity() ?
            Qt::transparent :
            mayoTheme()->color(Theme::Color::ButtonView3d_Background)
    ;

    auto btn = new ButtonFlat(parent);
    const double pxRatio = btn->devicePixelRatio();
    btn->setBackgroundBrush(bkgndColor);
    btn->setCheckedBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Checked));
    btn->setHoverBrush(mayoTheme()->color(Theme::Color::ButtonView3d_Hover));
    btn->setIcon(mayoTheme()->icon(icon));
    btn->setIconSize(pxRatio * QSize{24, 24});
    btn->setFixedSize(pxRatio * QSize{40, 40});
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
    const int margin = this->devicePixelRatio() * 2 * Internal_widgetMargin;
    auto fnGetViewControlsPos = [=]() -> QPoint {
        if (m_guiDoc->viewTrihedronMode() == GuiDocument::ViewTrihedronMode::AisViewCube) {
            const int viewCubeBndSize = m_guiDoc->aisViewCubeBoundingSize() / m_guiDoc->devicePixelRatio();
            if (m_guiDoc->viewTrihedronCorner() == Aspect_TOTP_LEFT_UPPER)
                return { margin, viewCubeBndSize + margin };
        }

        return { margin, margin };
    };

    m_widgetBtns->move(fnGetViewControlsPos());
}

} // namespace Mayo
