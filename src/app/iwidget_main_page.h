/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QWidget>

namespace Mayo {

class CommandContainer;

// Provides an interface for main pages within the Mayo application
// At its core, Mayo UI is basically a stack of widgets
// Such widgets are called "main pages" or just "pages" and only a single one is active at a time
class IWidgetMainPage : public QWidget {
    Q_OBJECT
public:
    // Builds UI objects(eg this might calls setupUi() on Qt-generated widgets)
    IWidgetMainPage(QWidget* parent = nullptr)
        : QWidget(parent)
    {}

    // Completes initialization of the page(eg by linking some buttons to existing commands)
    virtual void initialize(const CommandContainer* cmdContainer) = 0;

    // Update the activation("enabled" status) of the controls(ie any widget) belonging to this page
    virtual void updatePageControlsActivation() = 0;

signals:
    // Signal emitted when a "global" or "complete" activation at the whole application level is
    // required by this page
    // This might, for example, be consecutive to event internal to this page
    void updateGlobalControlsActivationRequired();
};

} // namespace Mayo
