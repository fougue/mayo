/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_grid.h"
#include "ui_widget_grid.h"
#include "property_editor_factory.h"
#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "../base/unit_system.h"
#include "../graphics/graphics_utils.h"

#include <Aspect_Grid.hxx>
#include <QtCore/QtDebug>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QColorDialog>
#include <limits>

namespace Mayo {

namespace {

QPixmap colorSquarePixmap(const Quantity_Color& color) {
    return IPropertyEditorFactory::colorSquarePixmap(QtGuiUtils::toQColor(color));
}

QPixmap colorSquarePixmap(const QColor& color) {
    return IPropertyEditorFactory::colorSquarePixmap(color);
}

} // namespace

WidgetGrid::WidgetGrid(GraphicsViewPtr viewPtr, QWidget* parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetGrid),
      m_viewPtr(viewPtr)
{
    const OccHandle<V3d_Viewer>& viewer = viewPtr->Viewer();

    // Intial configuration
    m_ui->setupUi(this);

    constexpr double maxFloat64 = std::numeric_limits<double>::max();
    const double valueEditorMaxWidth = m_ui->edit_RectSizeX->fontMetrics().averageCharWidth() * 15;
    for (auto editor : this->findChildren<QDoubleSpinBox*>()) {
        editor->setRange(-maxFloat64, maxFloat64);
        editor->setMaximumWidth(valueEditorMaxWidth);
    }

    m_ui->edit_RectRotation->setRange(-180, 180);
    m_ui->edit_CircRotation->setRange(-180, 180);
    m_ui->edit_RectStepX->setMinimum(0.);
    m_ui->edit_RectStepY->setMinimum(0.);
    m_ui->edit_RectSizeX->setMinimum(0.);
    m_ui->edit_RectSizeY->setMinimum(0.);
    m_ui->edit_CircRadiusStep->setMinimum(0.);
    m_ui->edit_CircRadius->setMinimum(0.);

    QtWidgetsUtils::collapseWidget(m_ui->widget_Config, true);
    QtWidgetsUtils::collapseWidget(m_ui->widget_Graphics, true);

    // Install grid visibility
    m_ui->check_ShowGrid->setChecked(GraphicsUtils::V3dViewer_isGridActive(viewer));

    // Install grid type
    switch (viewer->GridType()) {
    case Aspect_GT_Rectangular:
        m_ui->combo_Type->setCurrentIndex(0);
        m_ui->stack_Config->setCurrentWidget(m_ui->page_Rectangular);
        break;
    case Aspect_GT_Circular:
        m_ui->combo_Type->setCurrentIndex(1);
        m_ui->stack_Config->setCurrentWidget(m_ui->page_Circular);
        break;
    } // endswitch

    // Install rectangular grid params
    {
        double xOrigin, yOrigin;
        double xStep, yStep;
        double rotAngle;
        viewer->RectangularGridValues(xOrigin, yOrigin, xStep, yStep, rotAngle);
        m_ui->edit_RectOriginX->setValue(xOrigin);
        m_ui->edit_RectOriginY->setValue(yOrigin);
        m_ui->edit_RectStepX->setValue(xStep);
        m_ui->edit_RectStepY->setValue(yStep);
        m_ui->edit_RectRotation->setValue(UnitSystem::degrees(rotAngle * Quantity_Radian));

        double xSize, ySize;
        double offset;
        viewer->RectangularGridGraphicValues(xSize, ySize, offset);
        m_ui->edit_RectSizeX->setValue(xSize);
        m_ui->edit_RectSizeY->setValue(ySize);
        m_ui->edit_RectOffset->setValue(offset);
    }

    // Install circular grid params
    {
        double xOrigin, yOrigin;
        double radiusStep;
        int divisionCount;
        double rotAngle;
        viewer->CircularGridValues(xOrigin, yOrigin, radiusStep, divisionCount, rotAngle);
        m_ui->edit_CircOriginX->setValue(xOrigin);
        m_ui->edit_CircOriginY->setValue(yOrigin);
        m_ui->edit_CircRadiusStep->setValue(radiusStep);
        m_ui->edit_CircDivision->setValue(divisionCount);
        m_ui->edit_CircRotation->setValue(UnitSystem::degrees(rotAngle * Quantity_Radian));

        double radius;
        double offset;
        viewer->CircularGridGraphicValues(radius, offset);
        m_ui->edit_CircRadius->setValue(radius);
        m_ui->edit_CircOffset->setValue(offset);
    }

    // Install grid privileged plane
    const gp_Ax2 plane = viewer->PrivilegedPlane().Ax2();
    if (plane.IsCoplanar(gp::XOY(), Precision::Confusion(), Precision::Angular()))
        m_ui->combo_Plane->setCurrentIndex(0);
    else if (plane.IsCoplanar(gp::ZOX(), Precision::Confusion(), Precision::Angular()))
        m_ui->combo_Plane->setCurrentIndex(1);
    else if (plane.IsCoplanar(gp::YOZ(), Precision::Confusion(), Precision::Angular()))
        m_ui->combo_Plane->setCurrentIndex(2);
    else
        m_ui->combo_Plane->setCurrentIndex(3);

    // Install grid draw mode
    OccHandle<Aspect_Grid> gridAspect = GraphicsUtils::V3dViewer_grid(viewer);
    if (gridAspect) {
        if (gridAspect->DrawMode() == Aspect_GDM_Lines)
            m_ui->combo_DrawMode->setCurrentIndex(0);
        else if (gridAspect->DrawMode() == Aspect_GDM_Points)
            m_ui->combo_DrawMode->setCurrentIndex(1);
    }

    // Install grid draw colors
    auto gridColors = GraphicsUtils::V3dViewer_gridColors(viewer);
    m_ui->btn_Color->setIcon(colorSquarePixmap(gridColors.base));
    m_ui->btn_ColorTenth->setIcon(colorSquarePixmap(gridColors.tenth));
    m_gridColorTenth = gridColors.tenth;

    // Install widgets enable status
    m_ui->combo_Plane->setEnabled(GraphicsUtils::V3dViewer_isGridActive(viewer));
    m_ui->widget_Main->setEnabled(GraphicsUtils::V3dViewer_isGridActive(viewer));
    auto planeComboModel = static_cast<QStandardItemModel*>(m_ui->combo_Plane->model());
    if (planeComboModel)
        planeComboModel->item(3)->setFlags(Qt::NoItemFlags); // Custom plane
    else
        qWarning() << Q_FUNC_INFO << "QComboBox model isn't of type QStandardItemModel";

    // Signal/slot connections
    auto sigComboBoxActivated_int = qOverload<int>(&QComboBox::activated);
    QObject::connect(
        m_ui->check_ShowGrid, &QCheckBox::clicked, this, &WidgetGrid::activateGrid
    );
    QObject::connect(m_ui->combo_Type, sigComboBoxActivated_int, this, [=](int typeIndex) {
        m_ui->stack_Config->setCurrentIndex(typeIndex);
        auto gridColors = GraphicsUtils::V3dViewer_gridColors(viewer);
        viewer->ActivateGrid(
            toGridType(m_ui->combo_Type->currentIndex()),
            toGridDrawMode(m_ui->combo_DrawMode->currentIndex())
        );
        GraphicsUtils::V3dViewer_setGridColors(viewer, gridColors);
        m_viewPtr.redraw();
    });
    QObject::connect(m_ui->combo_Plane, sigComboBoxActivated_int, this, [=](int planeIndex) {
        viewer->SetPrivilegedPlane(toPlaneAxis(planeIndex));
        m_viewPtr.redraw();
    });
    QObject::connect(m_ui->combo_DrawMode, sigComboBoxActivated_int, this, [=](int modeIndex) {
        GraphicsUtils::V3dViewer_grid(viewer)->SetDrawMode(toGridDrawMode(modeIndex));
        m_viewPtr.redraw();
    });
    QObject::connect(m_ui->btn_Config, &QToolButton::clicked, this, [=](bool on) {
        QtWidgetsUtils::collapseWidget(m_ui->widget_Config, !on);
        if (on)
            m_ui->label_Type->setMinimumWidth(m_ui->label_RectOrigin->width());

        m_ui->btn_Config->setArrowType(on ? Qt::DownArrow : Qt::RightArrow);
        emit this->sizeAdjustmentRequested();
    });
    QObject::connect(m_ui->btn_Graphics, &QToolButton::clicked, this, [=](bool on) {
        QtWidgetsUtils::collapseWidget(m_ui->widget_Graphics, !on);
        m_ui->btn_Graphics->setArrowType(on ? Qt::DownArrow : Qt::RightArrow);
        emit this->sizeAdjustmentRequested();
    });
    QObject::connect(
        m_ui->btn_Color, &QToolButton::clicked, this, [=]{ this->chooseGridColor(GridColorType::Base); }
    );
    QObject::connect(
        m_ui->btn_ColorTenth, &QToolButton::clicked, this, [=]{ this->chooseGridColor(GridColorType::Tenth); }
    );
    QObject::connect(
        m_ui->check_ColorTenth, &QAbstractButton::toggled, this, &WidgetGrid::enableGridColorTenth
    );

    auto sigGridParamChanged_double = qOverload<double>(&QDoubleSpinBox::valueChanged);
    auto sigGridParamChanged_int = qOverload<int>(&QSpinBox::valueChanged);
    QObject::connect(m_ui->edit_RectOriginX, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_RectOriginY, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_RectStepX, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_RectStepY, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_RectRotation, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_RectSizeX, sigGridParamChanged_double, this, &WidgetGrid::applyGridGraphicsParams);
    QObject::connect(m_ui->edit_RectSizeY, sigGridParamChanged_double, this, &WidgetGrid::applyGridGraphicsParams);
    QObject::connect(m_ui->edit_RectOffset, sigGridParamChanged_double, this, &WidgetGrid::applyGridGraphicsParams);

    QObject::connect(m_ui->edit_CircOriginX, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_CircOriginY, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_CircRadiusStep, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_CircDivision, sigGridParamChanged_int, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_CircRotation, sigGridParamChanged_double, this, &WidgetGrid::applyGridParams);
    QObject::connect(m_ui->edit_CircRadius, sigGridParamChanged_double, this, &WidgetGrid::applyGridGraphicsParams);
    QObject::connect(m_ui->edit_CircOffset, sigGridParamChanged_double, this, &WidgetGrid::applyGridGraphicsParams);
}

WidgetGrid::~WidgetGrid()
{
    delete m_ui;
}

Aspect_GridType WidgetGrid::toGridType(int comboBoxItemIndex)
{
    return comboBoxItemIndex == 0 ? Aspect_GT_Rectangular : Aspect_GT_Circular;
}

Aspect_GridDrawMode WidgetGrid::toGridDrawMode(int comboBoxItemIndex)
{
    switch (comboBoxItemIndex) {
    case 0: return Aspect_GDM_Lines;
    case 1: return Aspect_GDM_Points;
    default: return Aspect_GDM_None;
    }
}

const gp_Ax2& WidgetGrid::toPlaneAxis(int comboBoxItemIndex)
{
    switch (comboBoxItemIndex) {
    case 0: return gp::XOY();
    case 1: return gp::ZOX();
    case 2: return gp::YOZ();
    default: return gp::XOY();
    }
}

void WidgetGrid::activateGrid(bool on)
{
    const OccHandle<V3d_Viewer>& viewer = m_viewPtr->Viewer();
    if (on) {
        viewer->ActivateGrid(
            toGridType(m_ui->combo_Type->currentIndex()),
            toGridDrawMode(m_ui->combo_DrawMode->currentIndex())
        );
    }
    else {
        viewer->DeactivateGrid();
    }

    m_viewPtr.redraw();
    m_ui->combo_Plane->setEnabled(on);
    m_ui->widget_Main->setEnabled(on);
}

void WidgetGrid::applyGridParams()
{
    auto fnCorrectedGridStep = [](double gridStep) {
        return !qFuzzyIsNull(gridStep) ? gridStep : 0.01;
    };
    const OccHandle<V3d_Viewer>& viewer = m_viewPtr->Viewer();
    auto gridType = toGridType(m_ui->combo_Type->currentIndex());
    if (gridType == Aspect_GT_Rectangular) {
        viewer->SetRectangularGridValues(
            m_ui->edit_RectOriginX->value(),
            m_ui->edit_RectOriginY->value(),
            fnCorrectedGridStep(m_ui->edit_RectStepX->value()),
            fnCorrectedGridStep(m_ui->edit_RectStepY->value()),
            UnitSystem::radians(m_ui->edit_RectRotation->value() * Quantity_Degree)
        );
    }
    else if (gridType == Aspect_GT_Circular) {
        viewer->SetCircularGridValues(
            m_ui->edit_CircOriginX->value(),
            m_ui->edit_CircOriginY->value(),
            fnCorrectedGridStep(m_ui->edit_CircRadiusStep->value()),
            m_ui->edit_CircDivision->value(),
            UnitSystem::radians(m_ui->edit_CircRotation->value() * Quantity_Degree)
        );
    }

    m_viewPtr.redraw();
}

void WidgetGrid::applyGridGraphicsParams()
{
    const OccHandle<V3d_Viewer>& viewer = m_viewPtr->Viewer();
    auto gridType = toGridType(m_ui->combo_Type->currentIndex());
    if (gridType == Aspect_GT_Rectangular) {
        viewer->SetRectangularGridGraphicValues(
            m_ui->edit_RectSizeX->value(), m_ui->edit_RectSizeY->value(), m_ui->edit_RectOffset->value()
        );
    }
    else if (gridType == Aspect_GT_Circular) {
        viewer->SetCircularGridGraphicValues(
            m_ui->edit_CircRadius->value(), m_ui->edit_RectOffset->value()
        );
    }

    m_viewPtr.redraw();
}

void WidgetGrid::chooseGridColor(GridColorType colorType)
{
    const OccHandle<V3d_Viewer>& viewer = m_viewPtr->Viewer();
    auto gridColors = GraphicsUtils::V3dViewer_gridColors(viewer);
    // Helper function to apply some base/tenth grid color
    auto fnApplyGridColor = [=](const Quantity_Color& color) {
        if (colorType == GridColorType::Base) {
            const auto colorTenth = m_ui->check_ColorTenth->isChecked() ? gridColors.tenth : color;
            GraphicsUtils::V3dViewer_setGridColors(viewer, { color, colorTenth });
        }
        else {
            GraphicsUtils::V3dViewer_setGridColors(viewer, { gridColors.base, color });
        }

        m_viewPtr.redraw();
    };

    // Setup dialog to select a color
    auto dlg = new QColorDialog(this);
    auto onEntryGridColor = colorType == GridColorType::Base ? gridColors.base : gridColors.tenth;
    dlg->setCurrentColor(QtGuiUtils::toQColor(onEntryGridColor));

    QObject::connect(dlg, &QColorDialog::currentColorChanged, this, [=](const QColor& color) {
        fnApplyGridColor(QtGuiUtils::toColor<Quantity_Color>(color));
    });
    QObject::connect(dlg, &QDialog::accepted, this, [=]{
        auto btn = colorType == GridColorType::Base ? m_ui->btn_Color : m_ui->btn_ColorTenth;
        btn->setIcon(colorSquarePixmap(dlg->selectedColor()));
        if (colorType == GridColorType::Tenth)
            m_gridColorTenth = QtGuiUtils::toColor<Quantity_Color>(dlg->selectedColor());
    });
    QObject::connect(dlg, &QDialog::rejected, this, [=]{
        auto btn = colorType == GridColorType::Base ? m_ui->btn_Color : m_ui->btn_ColorTenth;
        btn->setIcon(colorSquarePixmap(onEntryGridColor));
        fnApplyGridColor(onEntryGridColor);
    });

    QtWidgetsUtils::asyncDialogExec(dlg);
}

void WidgetGrid::enableGridColorTenth(bool on)
{
    const OccHandle<V3d_Viewer>& viewer = m_viewPtr->Viewer();
    m_ui->label_ColorTenth->setEnabled(on);
    m_ui->btn_ColorTenth->setEnabled(on);
    auto gridColors = GraphicsUtils::V3dViewer_gridColors(viewer);
    const auto gridColorTenth = on ? m_gridColorTenth : gridColors.base;
    GraphicsUtils::V3dViewer_setGridColors(viewer, { gridColors.base, gridColorTenth });
    m_viewPtr.redraw();
}

} // namespace Mayo
