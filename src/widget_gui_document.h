/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QWidget>

namespace Mayo {

class BaseV3dViewController;
class GuiDocument;
class WidgetClipPlanes;
class WidgetOccView;

class WidgetGuiDocument : public QWidget {
    Q_OBJECT
public:
    WidgetGuiDocument(GuiDocument* guiDoc, QWidget* parent = nullptr);

    GuiDocument* guiDocument() const;
    BaseV3dViewController* controller() const;

private:
    void toggleWidgetClipPlanes();

    GuiDocument* m_guiDoc = nullptr;
    BaseV3dViewController* m_controller = nullptr;
    WidgetOccView* m_qtOccView = nullptr;
    WidgetClipPlanes* m_widgetClipPlanes = nullptr;
    QRect m_firstBtnFrameRect;
};

} // namespace Mayo
