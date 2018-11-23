/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_shape_selector.h"
#include "gui_document.h"
#include "widget_gui_document.h"
#include "qt_occ_view_controller.h"

#include <AIS_InteractiveContext.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <StdSelect_ShapeTypeFilter.hxx>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>

#include <QtCore/QtDebug>

namespace Mayo {

static QString stringShapeTypePlural(TopAbs_ShapeEnum shapeType)
{
    switch (shapeType) {
    case TopAbs_VERTEX: return WidgetShapeSelector::tr("Vertices");
    case TopAbs_EDGE: return WidgetShapeSelector::tr("Edges");
    case TopAbs_WIRE: return WidgetShapeSelector::tr("Wires");
    case TopAbs_FACE: return WidgetShapeSelector::tr("Faces");
    case TopAbs_SHELL: return WidgetShapeSelector::tr("Shells");
    case TopAbs_SOLID: return WidgetShapeSelector::tr("Solids");
    }
    return QString("?");
}

static const TopAbs_ShapeEnum allShapeTypes[] = {
    TopAbs_VERTEX, TopAbs_EDGE, TopAbs_WIRE, TopAbs_FACE, TopAbs_SHELL, TopAbs_SOLID
};

WidgetShapeSelector::WidgetShapeSelector(WidgetGuiDocument* widgetGuiDoc)
    : WidgetShapeSelector(allShapeTypes, widgetGuiDoc)
{
}

WidgetShapeSelector::WidgetShapeSelector(
        Span<const TopAbs_ShapeEnum> spanShapeType,
        WidgetGuiDocument* widgetGuiDoc)
    : QWidget(widgetGuiDoc),
      m_widgetGuiDoc(widgetGuiDoc)
{
    Q_ASSERT(widgetGuiDoc);

    auto comboBox = new QComboBox(this);
    m_comboBoxShapeType = comboBox;
    for (TopAbs_ShapeEnum shapeType : spanShapeType)
        comboBox->addItem(tr("Select %1").arg(stringShapeTypePlural(shapeType)), shapeType);
    m_btnBox = new QDialogButtonBox(this);
    m_btnBox->setCenterButtons(true);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(comboBox);
    mainLayout->addWidget(m_btnBox);

    QObject::connect(
                comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
                this, &WidgetShapeSelector::onShapeSelectionModeChanged);
    QObject::connect(
                m_btnBox, &QDialogButtonBox::accepted,
                this, &WidgetShapeSelector::accepted);
    QObject::connect(
                m_btnBox, &QDialogButtonBox::rejected,
                this, &WidgetShapeSelector::rejected);
    QObject::connect(
                m_btnBox, &QDialogButtonBox::clicked,
                this, &WidgetShapeSelector::buttonClicked);
    QObject::connect(
                widgetGuiDoc->controller(), &BaseV3dViewController::mouseMoved,
                this, &WidgetShapeSelector::onView3dMouseMove);
    QObject::connect(
                widgetGuiDoc->controller(), &BaseV3dViewController::mouseClicked,
                this, &WidgetShapeSelector::onView3dMouseClicked);

    m_widgetGuiDoc->installEventFilter(this);
    this->onShapeSelectionModeChanged(comboBox->currentIndex());
    m_widgetGuiDoc->guiDocument()->setDetectionEnabled(true);
}

WidgetShapeSelector::~WidgetShapeSelector()
{
    m_widgetGuiDoc->guiDocument()->setDetectionEnabled(false);
    m_widgetGuiDoc->guiDocument()->aisInteractiveContext()->ClearSelected(true);
}

void WidgetShapeSelector::addButton(QDialogButtonBox::StandardButton stdBtn)
{
    m_btnBox->addButton(stdBtn);
}

void WidgetShapeSelector::addButton(QAbstractButton *btn, QDialogButtonBox::ButtonRole role)
{
    m_btnBox->addButton(btn, role);
}

QDialogButtonBox::ButtonRole WidgetShapeSelector::buttonRole(QAbstractButton *btn) const
{
    return m_btnBox->buttonRole(btn);
}

void WidgetShapeSelector::paintEvent(QPaintEvent*)
{
    WidgetGuiDocument::paintPanel(this);
}

void WidgetShapeSelector::onShapeSelectionModeChanged(int comboBoxItemId)
{
    bool okToInt = false;
    const int iShapeType =
            m_comboBoxShapeType->itemData(comboBoxItemId).toInt(&okToInt);
    const TopAbs_ShapeEnum shapeType =
            okToInt ? static_cast<TopAbs_ShapeEnum>(iShapeType) : TopAbs_SHAPE;
    if (shapeType != TopAbs_SHAPE) {
        const Handle_AIS_InteractiveContext& ctx =
                m_widgetGuiDoc->guiDocument()->aisInteractiveContext();
        ctx->ClearDetected(true);
        ctx->RemoveFilters();
        ctx->AddFilter(new StdSelect_ShapeTypeFilter(shapeType));
        emit selectionModeChanged(shapeType);
    }
}

void WidgetShapeSelector::onView3dMouseMove(const QPoint& pos)
{
    const BaseV3dViewController* ctrl = m_widgetGuiDoc->controller();
    const GuiDocument* guiDoc = m_widgetGuiDoc->guiDocument();
    if (!ctrl->isPanning() && !ctrl->isRotating()) {
        const Handle_AIS_InteractiveContext& ctx = guiDoc->aisInteractiveContext();
        ctx->MoveTo(pos.x(), pos.y(), guiDoc->v3dView(), true);
    }
}

void WidgetShapeSelector::onView3dMouseClicked(Qt::MouseButton btn)
{
    const Handle_AIS_InteractiveContext& ctx =
            m_widgetGuiDoc->guiDocument()->aisInteractiveContext();
    if (!ctx->HasDetected())
        ctx->ClearSelected(true);
    else
        ctx->ShiftSelect(true);
    if (btn == Qt::LeftButton && ctx->HasDetected()) {
        auto brepOwner = Handle_StdSelect_BRepOwner::DownCast(ctx->DetectedOwner());
        if (!brepOwner.IsNull() && brepOwner->HasShape())
            emit shapeClicked(brepOwner->Shape());
    }
}

} // namespace Mayo
