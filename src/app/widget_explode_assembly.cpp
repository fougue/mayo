/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_explode_assembly.h"
#include "ui_widget_explode_assembly.h"
#include "../base/bnd_utils.h"
#include "../base/math_utils.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_document.h"

#include <QtCore/QSignalBlocker>

namespace Mayo {

WidgetExplodeAssembly::WidgetExplodeAssembly(GuiDocument* guiDoc, QWidget* parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetExplodeAssembly),
      m_guiDoc(guiDoc)
{
    m_ui->setupUi(this);

    const DocumentPtr& doc = m_guiDoc->document();
    for (int i = 0; i < doc->entityCount(); ++i)
        this->mapEntity(doc->entityTreeNodeId(i));

    QObject::connect(doc.get(), &Document::entityAdded, this, &WidgetExplodeAssembly::mapEntity);
    QObject::connect(doc.get(), &Document::entityAboutToBeDestroyed, this, &WidgetExplodeAssembly::unmapEntity);

    QObject::connect(m_ui->slider_Factor, &QSlider::valueChanged, this, [=](int pct) {
        QSignalBlocker sigBlock(m_ui->edit_Factor);
        m_ui->edit_Factor->setValue(pct);
        this->onFactorChanged(pct / 100.);
    });
    QObject::connect(m_ui->edit_Factor, qOverload<int>(&QSpinBox::valueChanged), this, [=](int pct) {
        QSignalBlocker sigBlock(m_ui->slider_Factor);
        m_ui->slider_Factor->setValue(pct);
        this->onFactorChanged(pct / 100.);
    });
}

WidgetExplodeAssembly::~WidgetExplodeAssembly()
{
    delete m_ui;
}

void WidgetExplodeAssembly::onFactorChanged(double t)
{
    for (const Entity& entity : m_vecEntity) {
        const gp_Pnt entityCenter = BndBoxCoords::get(entity.bndBox).center();
        for (const Movable& movable : entity.vecMovable) {
            const gp_Vec vecDirection(entityCenter, BndBoxCoords::get(movable.bndBox).center());
            gp_Trsf trsfMove;
            trsfMove.SetTranslation(2 * t * vecDirection);
            AIS_InteractiveContext* context = GraphicsUtils::AisObject_contextPtr(movable.gfxObject);
            if (context)
                context->SetLocation(movable.gfxObject, trsfMove * movable.trsfOriginal);
        }
    }

    m_guiDoc->graphicsScene()->redraw();
}

void WidgetExplodeAssembly::mapEntity(TreeNodeId entityNodeId)
{
    Entity entity;
    entity.treeNodeId = entityNodeId;
    m_guiDoc->foreachGraphicsObject(entityNodeId, [&](GraphicsObjectPtr gfxObject) {
        Movable movable;
        movable.gfxObject = gfxObject;
        movable.bndBox = GraphicsUtils::AisObject_boundingBox(gfxObject);
        AIS_InteractiveContext* context = GraphicsUtils::AisObject_contextPtr(gfxObject);
        if (context)
            movable.trsfOriginal = context->Location(gfxObject);

        entity.vecMovable.push_back(std::move(movable));
        BndUtils::add(&entity.bndBox, entity.vecMovable.back().bndBox);
    });

    if (entity.vecMovable.size() > 1)
        m_vecEntity.push_back(std::move(entity));
}

void WidgetExplodeAssembly::unmapEntity(TreeNodeId entityNodeId)
{
    auto itFound = std::find_if(m_vecEntity.begin(), m_vecEntity.end(), [=](const Entity& entity) {
        return entity.treeNodeId == entityNodeId;
    });
    if (itFound != m_vecEntity.end())
        m_vecEntity.erase(itFound);
}

} // namespace Mayo
