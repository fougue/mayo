/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "widget_occ_view_controller.h"
#include <QtWidgets/QWidget>
#include <V3d_TypeOfOrientation.hxx>
#include <vector>

namespace Mayo {

class ButtonFlat;
class GuiDocument;
class WidgetClipPlanes;
class WidgetOccView;

class WidgetGuiDocument : public QWidget {
    Q_OBJECT
public:
    WidgetGuiDocument(GuiDocument* guiDoc, QWidget* parent = nullptr);

    GuiDocument* guiDocument() const { return m_guiDoc; }
    V3dViewController* controller() const { return m_controller; }

    static void paintPanel(QWidget* widget);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void toggleWidgetClipPlanes();
    void layoutWidgetClipPlanes();

    void recreateViewControls();
    QRect viewControlsRect() const;
    void layoutViewControls();

    GuiDocument* m_guiDoc = nullptr;
    WidgetOccView* m_qtOccView = nullptr;
    WidgetOccViewController* m_controller = nullptr;
    WidgetClipPlanes* m_widgetClipPlanes = nullptr;
    QRect m_rectControls;

    ButtonFlat* m_btnFitAll = nullptr;
    ButtonFlat* m_btnEditClipping = nullptr;
    std::vector<QWidget*> m_vecWidgetForViewProj;
};

} // namespace Mayo
