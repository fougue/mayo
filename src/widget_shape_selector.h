/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <TopAbs_ShapeEnum.hxx>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QWidget>
class QComboBox;
class TopoDS_Shape;

namespace Mayo {

class WidgetGuiDocument;

class WidgetShapeSelector : public QWidget {
    Q_OBJECT
public:
    WidgetShapeSelector(WidgetGuiDocument* widgetGuiDoc);
    WidgetShapeSelector(
            Span<const TopAbs_ShapeEnum> spanShapeType,
            WidgetGuiDocument* widgetGuiDoc);
    ~WidgetShapeSelector();

    void addButton(QDialogButtonBox::StandardButton stdBtn);
    void addButton(QAbstractButton* btn, QDialogButtonBox::ButtonRole role);
    QDialogButtonBox::ButtonRole buttonRole(QAbstractButton* btn) const;

signals:
    void selectionModeChanged(TopAbs_ShapeEnum shapeType);
    void shapeClicked(const TopoDS_Shape& shape);
    void buttonClicked(QAbstractButton* btn);
    void accepted();
    void rejected();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void onShapeSelectionModeChanged(int comboBoxItemId);
    void onView3dMouseMove(const QPoint& pos);
    void onView3dMouseClicked(Qt::MouseButton btn);

    Mayo::WidgetGuiDocument* m_widgetGuiDoc;
    QComboBox* m_comboBoxShapeType;
    QDialogButtonBox* m_btnBox;
};

} // namespace Mayo
