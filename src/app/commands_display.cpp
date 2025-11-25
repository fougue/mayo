/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_display.h"

#include "../base/application.h"
#include "../base/cpp_utils.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../gui/v3d_view_controller.h"
#include "../qtcommon/qstring_conv.h"
#include "app_module.h"
#include "theme.h"

#include <V3d_View.hxx>
#include <QtCore/QSignalBlocker>
#include <QActionGroup> // WARNING Qt5 <QtWidgets/...> / Qt6 <QtGui/...>
#include <QtWidgets/QMenu>

namespace Mayo {

BaseCommandDisplay::BaseCommandDisplay(IAppContext* context)
    : Command(context)
{
}

bool BaseCommandDisplay::getEnabledStatus() const
{
    return this->app()->documentCount() != 0
           && this->context()->currentPage() == IAppContext::Page::Documents
        ;
}

CommandChangeProjection::CommandChangeProjection(IAppContext* context)
    : BaseCommandDisplay(context)
{
    m_actionOrtho = new QAction(Command::tr("Orthographic"), this);
    m_actionPersp = new QAction(Command::tr("Perspective"), this);
    m_actionOrtho->setCheckable(true);
    m_actionPersp->setCheckable(true);

    auto menu = new QMenu(this->widgetMain());
    menu->addAction(m_actionOrtho);
    menu->addAction(m_actionPersp);

    auto group = new QActionGroup(menu);
    group->setExclusive(true);
    group->addAction(m_actionOrtho);
    group->addAction(m_actionPersp);

    auto action = new QAction(this);
    action->setText(Command::tr("Projection"));
    action->setMenu(menu);
    this->setAction(action);

    QObject::connect(group, &QActionGroup::triggered, this, [=](const QAction* action) {
        GuiDocument* guiDoc = this->currentGuiDocument();
        if (guiDoc) {
            guiDoc->v3dView()->Camera()->SetProjectionType(
                action == m_actionOrtho ?
                    Graphic3d_Camera::Projection_Orthographic :
                    Graphic3d_Camera::Projection_Perspective
            );
            guiDoc->graphicsView().redraw();
        }
    });

    QObject::connect(
        context, &IAppContext::currentDocumentChanged,
        this, &CommandChangeProjection::onCurrentDocumentChanged
    );
}

void CommandChangeProjection::execute()
{
}

void CommandChangeProjection::onCurrentDocumentChanged()
{
    const GuiDocument* guiDoc = this->currentGuiDocument();
    if (!guiDoc)
        return;

    // Sync menu with current projection type
    const auto viewProjType = guiDoc->v3dView()->Camera()->ProjectionType();
    Q_ASSERT(
        viewProjType == Graphic3d_Camera::Projection_Perspective
        || viewProjType == Graphic3d_Camera::Projection_Orthographic
    );
    QAction* actionProj = viewProjType == Graphic3d_Camera::Projection_Perspective ? m_actionPersp : m_actionOrtho;
    [[maybe_unused]] QSignalBlocker sigBlk(this->action());
    actionProj->setChecked(true);
}

CommandChangeDisplayMode::CommandChangeDisplayMode(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Mode"));
    this->setAction(action);
}

CommandChangeDisplayMode::CommandChangeDisplayMode(IAppContext* context, QMenu* containerMenu)
    : CommandChangeDisplayMode(context)
{
    QObject::connect(
        containerMenu, &QMenu::aboutToShow,
        this, &CommandChangeDisplayMode::recreateMenuDisplayMode
    );
}

void CommandChangeDisplayMode::execute()
{
}

void CommandChangeDisplayMode::recreateMenuDisplayMode()
{
    QMenu* menu = this->action()->menu();
    if (!menu) {
        menu = new QMenu(this->widgetMain());
        this->action()->setMenu(menu);
    }

    menu->clear();

    GuiDocument* guiDoc = this->currentGuiDocument();
    if (!guiDoc)
        return;

    const auto spanDrivers = this->guiApp()->graphicsObjectDrivers();
    for (const GraphicsObjectDriverPtr& driver : spanDrivers) {
        if (driver->displayModes().empty())
            continue; // Skip

        if (driver != spanDrivers.front())
            menu->addSeparator();

        const std::string driverTypeName = driver->DynamicType()->Name();
        const std::string trDriverTypeName{GraphicsObjectDriverI18N::textIdTr(driverTypeName)};

        auto group = new QActionGroup(menu);
        group->setExclusive(true);
        for (const Enumeration::Item& displayMode : driver->displayModes().items()) {
            const QString actionText =
                Command::tr("[%1] %2")
                    .arg(to_QString(trDriverTypeName))
                    .arg(to_QString(displayMode.name.tr()))
                ;
            auto action = new QAction(actionText, menu);
            action->setCheckable(true);
            action->setData(displayMode.value);
            menu->addAction(action);
            group->addAction(action);
            if (displayMode.value == guiDoc->activeDisplayMode(driver))
                action->setChecked(true);
        }

        QObject::connect(group, &QActionGroup::triggered, this, [=](QAction* action) {
            guiDoc->setActiveDisplayMode(driver, action->data().toInt());
            guiDoc->graphicsView().redraw();
        });
    }
}

CommandToggleOriginTrihedron::CommandToggleOriginTrihedron(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Show Origin Trihedron"));
    action->setToolTip(Command::tr("Show/Hide Origin Trihedron"));
    action->setCheckable(true);
    action->setChecked(false);
    this->setAction(action);

    QObject::connect(
        context, &IAppContext::currentDocumentChanged,
        this, &CommandToggleOriginTrihedron::onCurrentDocumentChanged
    );
    context->guiApp()->signalGuiDocumentOriginTrihedronVisibilityToggled.connectSlot(
        [=](GuiDocument* guiDoc, bool on) {
            if (guiDoc->documentIdentifier() == context->currentDocument())
                action->setChecked(on);
        }
    );
}

void CommandToggleOriginTrihedron::execute()
{
    GuiDocument* guiDoc = this->currentGuiDocument();
    if (guiDoc) {
        guiDoc->toggleOriginTrihedronVisibility();
        guiDoc->graphicsScene()->redraw();
    }
}

void CommandToggleOriginTrihedron::onCurrentDocumentChanged()
{
    GuiDocument* guiDoc = this->currentGuiDocument();
    if (guiDoc) {
        // Sync action with current visibility status of origin trihedron
        [[maybe_unused]] QSignalBlocker sigBlk(this->action());
        this->action()->setChecked(guiDoc->isOriginTrihedronVisible());
    }
    else {
        this->action()->setChecked(false);
    }
}

CommandTogglePerformanceStats::CommandTogglePerformanceStats(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Show Performance Stats"));
    action->setToolTip(Command::tr("Show/Hide rendering performance statistics"));
    action->setCheckable(true);
    action->setChecked(false);
    this->setAction(action);

    QObject::connect(
        context, &IAppContext::currentDocumentChanged,
        this, &CommandTogglePerformanceStats::onCurrentDocumentChanged
    );
}

void CommandTogglePerformanceStats::execute()
{
    GuiDocument* guiDoc = this->currentGuiDocument();
    if (guiDoc) {
        CppUtils::toggle(guiDoc->graphicsView()->ChangeRenderingParams().ToShowStats);
        guiDoc->graphicsView().redraw();
    }
}

void CommandTogglePerformanceStats::onCurrentDocumentChanged()
{
    GuiDocument* guiDoc = this->currentGuiDocument();
    if (guiDoc) {
        // Sync action with current visibility status of rendering performance stats
        [[maybe_unused]] QSignalBlocker sigBlk(this->action());
        this->action()->setChecked(guiDoc->v3dView()->ChangeRenderingParams().ToShowStats);
    }
    else {
        this->action()->setChecked(false);
    }
}

CommandZoomInCurrentDocument::CommandZoomInCurrentDocument(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Zoom In"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::ZoomIn));
    action->setShortcut(Qt::CTRL | Qt::Key_Plus);
    this->setAction(action);
}

void CommandZoomInCurrentDocument::execute()
{
    auto ctrl = this->context()->v3dViewController(this->currentGuiDocument());
    if (ctrl)
        ctrl->zoomIn();
}

CommandZoomOutCurrentDocument::CommandZoomOutCurrentDocument(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Zoom Out"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::ZoomOut));
    action->setShortcut(Qt::CTRL | Qt::Key_Minus);
    this->setAction(action);
}

void CommandZoomOutCurrentDocument::execute()
{
    auto ctrl = this->context()->v3dViewController(this->currentGuiDocument());
    if (ctrl)
        ctrl->zoomOut();
}

CommandTurnViewCounterClockWise::CommandTurnViewCounterClockWise(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Turn Counter Clockwise"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::TurnCounterClockwise));
    action->setShortcut(Qt::CTRL | Qt::Key_Left);
    this->setAction(action);
}

void CommandTurnViewCounterClockWise::execute()
{
    const QuantityAngle increment = AppModule::get()->properties()->turnViewAngleIncrement.quantity();
    auto ctrl = this->context()->v3dViewController(this->currentGuiDocument());
    if (ctrl)
        ctrl->turn(V3d_Z, -increment);
}

CommandTurnViewClockWise::CommandTurnViewClockWise(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Turn Clockwise"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::TurnClockwise));
    action->setShortcut(Qt::CTRL | Qt::Key_Right);
    this->setAction(action);
}

void CommandTurnViewClockWise::execute()
{
    const QuantityAngle increment = AppModule::get()->properties()->turnViewAngleIncrement.quantity();
    auto ctrl = this->context()->v3dViewController(this->currentGuiDocument());
    if (ctrl)
        ctrl->turn(V3d_Z, increment);
}

} // namespace Mayo
