/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "iwidget_main_page.h"

namespace Mayo {

// Provides the home page of the application
// Gives access to recent files as a 2D grid
class WidgetMainHome : public IWidgetMainPage {
    Q_OBJECT
public:
    WidgetMainHome(QWidget* parent = nullptr);
    ~WidgetMainHome();

    void initialize(const CommandContainer* cmdContainer) override;
    void updatePageControlsActivation() override;

private:
    class Ui_WidgetMainHome* m_ui = nullptr;
};

} // namespace Mayo
