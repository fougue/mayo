/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iwidget_main_page.h"

namespace Mayo {

// Provides the home page of the application
// Gives access to recent files as a 2D grid
class WidgetMainHome : public IWidgetMainPage {
    Q_OBJECT
public:
    explicit WidgetMainHome(QWidget* parent = nullptr);
    ~WidgetMainHome();

    void initialize(const CommandContainer* cmdContainer) override;
    void updatePageControlsActivation() override;

    void restoreUiState(const AppUiState&) override;
    void saveUiState(AppUiState&) override;

private:
    class Ui_WidgetMainHome* m_ui = nullptr;
};

} // namespace Mayo
