/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtWidgets/QWidget>

namespace Mayo {

class GuiDocument;

// Widget panel dedicated to exploding of assemblies within a GuiDocument object
class WidgetExplodeAssembly : public QWidget {
    Q_OBJECT
public:
    WidgetExplodeAssembly(GuiDocument* guiDoc, QWidget* parent = nullptr);
    ~WidgetExplodeAssembly();

private:
    class Ui_WidgetExplodeAssembly* m_ui= nullptr;
    GuiDocument* m_guiDoc = nullptr;
};

} // namespace Mayo
