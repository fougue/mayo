/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "commands_view.h"

#include "../base/application_item.h"
#include "../base/document.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../graphics/graphics_utils.h"
#include "theme.h"

#include <QtGui/QKeySequence>

#include <V3d_View.hxx>

#include <map>

namespace Mayo {

namespace {

// Data to display for a standard view orientation
struct ViewOrientationData {
    V3d_TypeOfOrientation value;
    Theme::Icon icon;
    const char* label;
};

constexpr ViewOrientationData viewOrientationData[] = {
    { V3d_XposYnegZpos, Theme::Icon::View3dIso,    "Isometric" },
    { V3d_Yneg,         Theme::Icon::View3dFront,  "Front" },
    { V3d_Ypos,         Theme::Icon::View3dBack,   "Back" },
    { V3d_Xneg,         Theme::Icon::View3dLeft,   "Left" },
    { V3d_Xpos,         Theme::Icon::View3dRight,  "Right" },
    { V3d_Zpos,         Theme::Icon::View3dTop,    "Top" },
    { V3d_Zneg,         Theme::Icon::View3dBottom, "Bottom" }
};

const ViewOrientationData* findViewOrientationData(V3d_TypeOfOrientation orientation)
{
    for (const ViewOrientationData& data : viewOrientationData) {
        if (data.value == orientation)
            return &data;
    }

    return nullptr;
}

// Returns the identifiers of the document tree nodes selected in 'guiDoc'
std::vector<TreeNodeId> selectedNodeIds(const GuiDocument* guiDoc)
{
    std::vector<TreeNodeId> vecNodeId;
    const GuiApplication* guiApp = guiDoc ? guiDoc->guiApplication() : nullptr;
    if (!guiApp || !guiApp->selectionModel())
        return vecNodeId;

    for (const ApplicationItem& appItem : guiApp->selectionModel()->selectedItems()) {
        if (appItem.isDocumentTreeNode() && appItem.document() == guiDoc->document())
            vecNodeId.push_back(appItem.documentTreeNode().id());
    }

    return vecNodeId;
}

// Visibility state of the document nodes, as recorded for undo/redo
struct VisibilityState {
    DocumentPtr document;
    std::map<TreeNodeId, CheckState> nodeState;
};

VisibilityState recordVisibilityState(const GuiDocument* guiDoc)
{
    VisibilityState state;
    if (!guiDoc)
        return state;

    state.document = guiDoc->document();
    for (TreeNodeId nodeId : state.document->allEntityNodeIds())
        state.nodeState.insert({ nodeId, guiDoc->nodeVisibleState(nodeId) });
    return state;
}

void applyVisibilityState(GuiDocument* guiDoc, const VisibilityState& state)
{
    if (!guiDoc)
        return;

    for (const auto& [nodeId, nodeState] : state.nodeState)
        guiDoc->setNodeVisible(nodeId, nodeState != CheckState::Off);
    guiDoc->graphicsScene()->redraw();
}

// History of the visibility operations, shared by the undo/redo commands
class VisibilityHistory {
public:
    static VisibilityHistory& instance()
    {
        static VisibilityHistory history;
        return history;
    }

    void record(const GuiDocument* guiDoc)
    {
        if (!guiDoc)
            return;

        m_vecUndo.push_back(recordVisibilityState(guiDoc));
        if (m_vecUndo.size() > MaxSize)
            m_vecUndo.erase(m_vecUndo.begin());
        m_vecRedo.clear();
    }

    bool canUndo() const { return !m_vecUndo.empty(); }
    bool canRedo() const { return !m_vecRedo.empty(); }

    void undo(const GuiApplication* guiApp)
    {
        if (!m_vecUndo.empty())
            this->restore(guiApp, m_vecUndo, m_vecRedo);
    }

    void redo(const GuiApplication* guiApp)
    {
        if (!m_vecRedo.empty())
            this->restore(guiApp, m_vecRedo, m_vecUndo);
    }

private:
    constexpr static std::size_t MaxSize = 32;

    using StateVector = std::vector<VisibilityState>;

    // Restores the last state of 'vecFrom' and records the current one into 'vecTo'
    static void restore(const GuiApplication* guiApp, StateVector& vecFrom, StateVector& vecTo)
    {
        VisibilityState state = std::move(vecFrom.back());
        vecFrom.pop_back();

        GuiDocument* guiDoc = guiApp ? guiApp->findGuiDocument(state.document) : nullptr;
        if (!guiDoc)
            return;

        vecTo.push_back(recordVisibilityState(guiDoc));
        applyVisibilityState(guiDoc, state);
    }

    StateVector m_vecUndo;
    StateVector m_vecRedo;
};

} // namespace

// --
// -- CommandFitAll
// --

CommandFitAll::CommandFitAll(IAppContext* context)
    : BaseCommandDisplay(context)
{
    auto action = this->createAction();
    action->setText(Command::tr("Fit All"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::Expand));
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
}

void CommandFitAll::execute()
{
    GuiDocument* guiDoc = this->currentGuiDocument();
    if (!guiDoc)
        return;

    guiDoc->runViewCameraAnimation([=](const OccHandle<V3d_View>& view) {
        constexpr int flags = GuiDocument::OnlySelectedGraphics | GuiDocument::OnlyVisibleGraphics;
        GraphicsUtils::V3dView_fitAll(view, guiDoc->graphicsBoundingBox(flags));
    });
}

// --
// -- CommandSetViewOrientation
// --

CommandSetViewOrientation::CommandSetViewOrientation(
        IAppContext* context, V3d_TypeOfOrientation orientation
    )
    : BaseCommandDisplay(context),
      m_orientation(orientation)
{
    const ViewOrientationData* data = findViewOrientationData(orientation);
    auto action = this->createAction();
    if (data) {
        action->setText(Command::tr(data->label));
        action->setIcon(mayoTheme()->icon(data->icon));
    }
}

void CommandSetViewOrientation::execute()
{
    if (GuiDocument* guiDoc = this->currentGuiDocument())
        guiDoc->setViewCameraOrientation(m_orientation, GuiDocument::ViewOrientationFlag_FitAll);
}

std::string_view CommandSetViewOrientation::name(V3d_TypeOfOrientation orientation)
{
    const ViewOrientationData* data = findViewOrientationData(orientation);
    if (!data)
        return {};

    switch (data->value) {
    case V3d_XposYnegZpos: return "current-doc-view-iso";
    case V3d_Yneg: return "current-doc-view-front";
    case V3d_Ypos: return "current-doc-view-back";
    case V3d_Xneg: return "current-doc-view-left";
    case V3d_Xpos: return "current-doc-view-right";
    case V3d_Zpos: return "current-doc-view-top";
    case V3d_Zneg: return "current-doc-view-bottom";
    default: return {};
    }
}

// --
// -- CommandShowHide
// --

CommandShowHide::CommandShowHide(IAppContext* context, Operation operation)
    : BaseCommandDisplay(context),
      m_operation(operation)
{
    auto action = this->createAction();
    switch (operation) {
    case Operation::ShowAll:
        action->setText(Command::tr("Show all"));
        action->setIcon(mayoTheme()->icon(Theme::Icon::VisibilityShowAll));
        action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
        break;
    case Operation::ShowSelection:
        action->setText(Command::tr("Show selection"));
        action->setIcon(mayoTheme()->icon(Theme::Icon::VisibilityShowSelection));
        break;
    case Operation::HideSelection:
        action->setText(Command::tr("Hide selection"));
        action->setIcon(mayoTheme()->icon(Theme::Icon::VisibilityHideSelection));
        action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
        break;
    case Operation::ShowOnlySelection:
        action->setText(Command::tr("Show only selection"));
        action->setIcon(mayoTheme()->icon(Theme::Icon::VisibilityShowSelectionOnly));
        action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
        break;
    }
}

void CommandShowHide::execute()
{
    GuiDocument* guiDoc = this->currentGuiDocument();
    if (!guiDoc)
        return;

    const DocumentPtr& doc = guiDoc->document();
    const std::vector<TreeNodeId> vecSelectedNodeId = selectedNodeIds(guiDoc);

    VisibilityHistory::instance().record(guiDoc);
    switch (m_operation) {
    case Operation::ShowAll:
        for (TreeNodeId nodeId : doc->allEntityNodeIds())
            guiDoc->setNodeVisible(nodeId, true);
        break;
    case Operation::ShowSelection:
        for (TreeNodeId nodeId : vecSelectedNodeId)
            guiDoc->setNodeVisible(nodeId, true);
        break;
    case Operation::HideSelection:
        for (TreeNodeId nodeId : vecSelectedNodeId)
            guiDoc->setNodeVisible(nodeId, false);
        break;
    case Operation::ShowOnlySelection:
        for (TreeNodeId nodeId : doc->allEntityNodeIds())
            guiDoc->setNodeVisible(nodeId, false);
        for (TreeNodeId nodeId : vecSelectedNodeId)
            guiDoc->setNodeVisible(nodeId, true);
        break;
    }

    guiDoc->graphicsScene()->redraw();
}

bool CommandShowHide::getEnabledStatus() const
{
    if (!BaseCommandDisplay::getEnabledStatus())
        return false;

    // All the operations but "show all" require a selection
    return m_operation == Operation::ShowAll
        || !selectedNodeIds(this->currentGuiDocument()).empty();
}

std::string_view CommandShowHide::name(Operation operation)
{
    switch (operation) {
    case Operation::ShowAll: return "current-doc-show-all";
    case Operation::ShowSelection: return "current-doc-show-selection";
    case Operation::HideSelection: return "current-doc-hide-selection";
    case Operation::ShowOnlySelection: return "current-doc-show-only-selection";
    }

    return {};
}

// --
// -- CommandUndoRedoVisibility
// --

CommandUndoRedoVisibility::CommandUndoRedoVisibility(IAppContext* context, Operation operation)
    : BaseCommandDisplay(context),
      m_operation(operation)
{
    auto action = this->createAction();
    if (operation == Operation::Undo) {
        action->setText(Command::tr("Undo visibility change"));
        action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    }
    else {
        action->setText(Command::tr("Redo visibility change"));
        action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    }
}

void CommandUndoRedoVisibility::execute()
{
    if (m_operation == Operation::Undo)
        VisibilityHistory::instance().undo(this->guiApp());
    else
        VisibilityHistory::instance().redo(this->guiApp());

    if (GuiDocument* guiDoc = this->currentGuiDocument())
        guiDoc->graphicsScene()->redraw();
}

bool CommandUndoRedoVisibility::getEnabledStatus() const
{
    if (!BaseCommandDisplay::getEnabledStatus())
        return false;

    const VisibilityHistory& history = VisibilityHistory::instance();
    return m_operation == Operation::Undo ? history.canUndo() : history.canRedo();
}

std::string_view CommandUndoRedoVisibility::name(Operation operation)
{
    return operation == Operation::Undo
        ? "current-doc-undo-visibility"
        : "current-doc-redo-visibility";
}

} // namespace Mayo
