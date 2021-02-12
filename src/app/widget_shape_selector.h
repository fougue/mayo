/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#if 0
#include "../base/span.h"
#include "../graphics/graphics_scene.h"
#include <TopAbs_ShapeEnum.hxx>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QWidget>
class QComboBox;
class TopoDS_Shape;

namespace Mayo {

class V3dViewController;
class GuiDocument;

class GpxShapeSelector : public QObject {
    Q_OBJECT
public:
    GpxShapeSelector(GuiDocument* guiDoc);
    ~GpxShapeSelector();

    V3dViewController* viewController() const;
    void setViewController(V3dViewController* ctrl);

    TopAbs_ShapeEnum shapeType() const;
    void setShapeType(TopAbs_ShapeEnum shapeEnum);

    enum class Mode {
        None,
        Single,
        Multi
    };

    Mode mode() const;
    void setMode(Mode mode);

    void clearSelection();

signals:
    void shapeTypeChanged(TopAbs_ShapeEnum shapeType);
    void shapeClicked(const TopoDS_Shape& shape);
    void shapeSelectionCleared();

protected:
    virtual void onShapeTypeChanged(TopAbs_ShapeEnum shapeEnum);
    virtual void onView3dMouseMove(const QPoint& pos);
    virtual void onView3dMouseClicked(Qt::MouseButton btn);

    const GraphicsScene* graphicsScene() const;

    bool hasSelectedShapes() const;

    GuiDocument* m_guiDocument = nullptr;
    V3dViewController* m_viewCtrl = nullptr;
    TopAbs_ShapeEnum m_shapeType;
    Mode m_mode = Mode::Multi;
};

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

    GpxShapeSelector* selector() const;

signals:
    void buttonClicked(QAbstractButton* btn);
    void accepted();
    void rejected();

private:
    void onShapeSelectionModeChanged(int comboBoxItemId);

    Mayo::WidgetGuiDocument* m_widgetGuiDoc;
    QComboBox* m_comboBoxShapeType;
    QDialogButtonBox* m_btnBox;
    GpxShapeSelector* m_selector;
};

} // namespace Mayo
#endif
