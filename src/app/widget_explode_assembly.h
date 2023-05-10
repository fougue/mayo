/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
