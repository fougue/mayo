/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_display.h"

#include <V3d_TypeOfOrientation.hxx>

#include <string_view>

namespace Mayo {

// Applies the "fit all" camera animation on the current document
class CommandFitAll : public BaseCommandDisplay {
public:
    explicit CommandFitAll(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "current-doc-fit-all";
};

// Sets the camera of the current document to a standard orientation
class CommandSetViewOrientation : public BaseCommandDisplay {
public:
    CommandSetViewOrientation(IAppContext* context, V3d_TypeOfOrientation orientation);
    void execute() override;

    // Identifier of the command applying the view 'orientation'(eg "current-doc-view-front")
    static std::string_view name(V3d_TypeOfOrientation orientation);

private:
    V3d_TypeOfOrientation m_orientation;
};

// Applies a visibility operation to the current document
class CommandShowHide : public BaseCommandDisplay {
public:
    enum class Operation {
        ShowAll,
        ShowSelection,
        HideSelection,
        ShowOnlySelection
    };

    CommandShowHide(IAppContext* context, Operation operation);
    void execute() override;
    bool getEnabledStatus() const override;

    // Identifier of the command applying 'operation'(eg "current-doc-hide-selection")
    static std::string_view name(Operation operation);

private:
    Operation m_operation;
};

// Undo/redo of the visibility operations
// NOTE Mayo has no global undo system : the history is limited to the visibility state of the
// document nodes, which is the only thing modified by the visibility commands
class CommandUndoRedoVisibility : public BaseCommandDisplay {
public:
    enum class Operation { Undo, Redo };

    CommandUndoRedoVisibility(IAppContext* context, Operation operation);
    void execute() override;
    bool getEnabledStatus() const override;

    // Identifier of the command applying 'operation'
    static std::string_view name(Operation operation);

private:
    Operation m_operation;
};

} // namespace Mayo
