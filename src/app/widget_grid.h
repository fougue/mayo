/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../graphics/graphics_view_ptr.h"

#include <QtWidgets/QWidget>
class gp_Ax2;

namespace Mayo {

// Widget panel dedicated to management of the grid in 3D view
class WidgetGrid : public QWidget {
    Q_OBJECT
public:
    WidgetGrid(GraphicsViewPtr viewPtr, QWidget* parent = nullptr);
    ~WidgetGrid();

signals:
    void sizeAdjustmentRequested();

private:
    enum class GridColorType { Base, Tenth };

    static Aspect_GridType toGridType(int comboBoxItemIndex);
    static Aspect_GridDrawMode toGridDrawMode(int comboBoxItemIndex);
    static const gp_Ax2& toPlaneAxis(int comboBoxItemIndex);

    void activateGrid(bool on);
    void applyGridParams();
    void applyGridGraphicsParams();
    void chooseGridColor(GridColorType colorType);
    void enableGridColorTenth(bool on);

    class Ui_WidgetGrid* m_ui = nullptr;
    GraphicsViewPtr m_viewPtr;
    Quantity_Color m_gridColorTenth;
};

} // namespace Mayo
