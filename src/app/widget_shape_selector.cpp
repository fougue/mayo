/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_shape_selector.h"

#if 0
#include "../gui/gui_document.h"
#include "widget_gui_document.h"
#include "widget_occ_view_controller.h"

#include <AIS_InteractiveContext.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <StdSelect_ShapeTypeFilter.hxx>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QShortcut>

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
    case TopAbs_COMPOUND: return WidgetShapeSelector::tr("Compounds");
    case TopAbs_COMPSOLID: return WidgetShapeSelector::tr("Connected solids");
    case TopAbs_SHAPE: return WidgetShapeSelector::tr("?Shapes?");
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
      m_widgetGuiDoc(widgetGuiDoc),
      m_selector(new GpxShapeSelector(widgetGuiDoc->guiDocument()))
{
    Q_ASSERT(widgetGuiDoc);

    this->setAutoFillBackground(true);
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
                comboBox, QOverload<int>::of(&QComboBox::activated),
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
    m_selector->setViewController(widgetGuiDoc->controller());

    auto escShortcut = new QShortcut(Qt::Key_Escape, this);
    QObject::connect(escShortcut, &QShortcut::activated, this, &WidgetShapeSelector::rejected);

    this->onShapeSelectionModeChanged(comboBox->currentIndex());
}

WidgetShapeSelector::~WidgetShapeSelector()
{
    delete m_selector;
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

GpxShapeSelector* WidgetShapeSelector::selector() const
{
    return m_selector;
}

void WidgetShapeSelector::onShapeSelectionModeChanged(int comboBoxItemId)
{
    bool okToInt = false;
    const int iShapeType =
            m_comboBoxShapeType->itemData(comboBoxItemId).toInt(&okToInt);
    const TopAbs_ShapeEnum shapeType =
            okToInt ? static_cast<TopAbs_ShapeEnum>(iShapeType) : TopAbs_SHAPE;
    m_selector->setShapeType(shapeType);
}


// --
// -- ShapeSelector
// --

GpxShapeSelector::GpxShapeSelector(GuiDocument* guiDoc)
    : QObject(guiDoc),
      m_guiDocument(guiDoc),
      m_shapeType(TopAbs_SHAPE)
{
    this->context()->RemoveFilters();
}

GpxShapeSelector::~GpxShapeSelector()
{
    this->clearSelection();
    this->context()->RemoveFilters();
}

V3dViewController* GpxShapeSelector::viewController() const
{
    return m_viewCtrl;
}

void GpxShapeSelector::setViewController(V3dViewController *ctrl)
{
    if (ctrl == m_viewCtrl)
        return;

    m_viewCtrl = ctrl;
    QObject::connect(
                ctrl, &V3dViewController::mouseMoved,
                this, &GpxShapeSelector::onView3dMouseMove);
    QObject::connect(
                ctrl, &V3dViewController::mouseClicked,
                this, &GpxShapeSelector::onView3dMouseClicked);
}

TopAbs_ShapeEnum GpxShapeSelector::shapeType() const
{
    return m_shapeType;
}

void GpxShapeSelector::setShapeType(TopAbs_ShapeEnum shapeEnum)
{
    m_shapeType = shapeEnum;
    this->onShapeTypeChanged(shapeEnum);
    emit shapeTypeChanged(shapeEnum);
}

GpxShapeSelector::Mode GpxShapeSelector::mode() const
{
    return m_mode;
}

void GpxShapeSelector::setMode(GpxShapeSelector::Mode mode)
{
    m_mode = mode;
    this->clearSelection();
}

const GraphicsScene* GpxShapeSelector::graphicsScene() const
{
    return m_guiDocument->graphicsScene();
}

bool GpxShapeSelector::hasSelectedShapes() const
{
    auto gfxScene = m_guiDocument->graphicsScene();
    auto foundOwner = gfxScene->findSelectedGraphicsOwner([](const GraphicsObjectPtr& ptr) {
        auto brepOwner = Handle_StdSelect_BRepOwner::DownCast(ptr);
        return !brepOwner.IsNull() && brepOwner->HasShape();
    });
    return !foundOwner.IsNull();
}

void GpxShapeSelector::clearSelection()
{
    this->graphicsScene()->clearSelection();
    emit shapeSelectionCleared();
}

void GpxShapeSelector::onShapeTypeChanged(TopAbs_ShapeEnum shapeEnum)
{
    this->clearSelection();
    this->context()->RemoveFilters();
    if (shapeEnum != TopAbs_SHAPE)
        this->context()->AddFilter(new StdSelect_ShapeTypeFilter(shapeEnum));
}

void GpxShapeSelector::onView3dMouseMove(const QPoint& pos)
{
    this->graphicsScene()->highlightAt(pos, m_guiDocument->v3dView());
}

void GpxShapeSelector::onView3dMouseClicked(Qt::MouseButton btn)
{
    const bool hadSelected = this->hasSelectedShapes();
    auto detectedEntity = Handle_StdSelect_BRepOwner::DownCast(this->context()->DetectedOwner());
    AIS_StatusOfPick pickStatus = AIS_SOP_NothingSelected;
    if (!this->context()->HasDetected()) {
        this->context()->ClearSelected(true);
    }
    else {
        if (m_mode == Mode::Single)
            pickStatus = this->context()->Select(true);
        else if (m_mode == Mode::Multi)
            pickStatus = this->context()->ShiftSelect(true);
    }

    if (m_mode == Mode::Multi && hadSelected && !this->hasSelectedShapes())
        emit shapeSelectionCleared();

    if (pickStatus == AIS_SOP_Error
            || pickStatus == AIS_SOP_NothingSelected
            || pickStatus == AIS_SOP_Removed)
    {
        return;
    }

    if (btn == Qt::LeftButton
            && !detectedEntity.IsNull()
            && detectedEntity->HasShape())
    {
        emit shapeClicked(detectedEntity->Shape());
    }
}

} // namespace Mayo
#endif
