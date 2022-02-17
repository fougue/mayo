/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "v3d_view_controller.h"

#include <QtCore/QDebug>
#include <QtCore/QRect>
#include <V3d_View.hxx>

namespace Mayo {

V3dViewController::V3dViewController(const Handle_V3d_View& view, QObject* parent)
    : QObject(parent),
      m_view(view)
{
}

V3dViewController::~V3dViewController()
{
    delete m_rubberBand;
}

void V3dViewController::zoomIn()
{
    m_view->SetScale(m_view->Scale() * 1.1); // +10%
    this->redrawView();
    emit viewScaled();
}

void V3dViewController::zoomOut()
{
    m_view->SetScale(m_view->Scale() / 1.1); // -10%
    this->redrawView();
    emit viewScaled();
}

void V3dViewController::startDynamicAction(DynamicAction dynAction)
{
    if (dynAction == DynamicAction::None)
        return;

    if (m_dynamicAction != DynamicAction::None)
        return;

    m_dynamicAction = dynAction;
    emit dynamicActionStarted(dynAction);
}

void V3dViewController::stopDynamicAction()
{
    if (m_dynamicAction != DynamicAction::None) {
        emit dynamicActionEnded(m_dynamicAction);
        m_dynamicAction = DynamicAction::None;
    }
}

bool V3dViewController::isRotationStarted() const
{
    return m_dynamicAction == DynamicAction::Rotation;
}

bool V3dViewController::isPanningStarted() const
{
    return m_dynamicAction == DynamicAction::Panning;
}

bool V3dViewController::isWindowZoomingStarted() const
{
    return m_dynamicAction == DynamicAction::WindowZoom;
}

void V3dViewController::rotation(const QPoint& currPos)
{
    if (!this->isRotationStarted()) {
        this->startDynamicAction(DynamicAction::Rotation);
        m_view->StartRotation(currPos.x(), currPos.y());
    }
    else {
        m_view->Rotation(currPos.x(), currPos.y());
        this->redrawView();
    }
}

void V3dViewController::pan(const QPoint& prevPos, const QPoint& currPos)
{
    if (!this->isPanningStarted())
        this->startDynamicAction(DynamicAction::Panning);

    m_view->Pan(currPos.x() - prevPos.x(), prevPos.y() - currPos.y());
    this->redrawView();
}

void V3dViewController::windowFitAll(const QPoint& posMin, const QPoint& posMax)
{
    if (std::abs(posMin.x() - posMax.x()) > 1 || std::abs(posMin.y() - posMax.y()) > 1)
        m_view->WindowFitAll(posMin.x(), posMin.y(), posMax.x(), posMax.y());
}

void V3dViewController::windowZoomRubberBand(const QPoint& currPos)
{
    if (!this->isWindowZoomingStarted()) {
        this->startDynamicAction(DynamicAction::WindowZoom);
        m_posRubberBandStart = currPos;
    }

    this->drawRubberBand(m_posRubberBandStart, currPos);
}

void V3dViewController::windowZoom(const QPoint& currPos)
{
    this->windowFitAll(m_posRubberBandStart, currPos);
    this->hideRubberBand();
}

void V3dViewController::startInstantZoom(const QPoint& currPos)
{
    this->startDynamicAction(DynamicAction::InstantZoom);
    this->backupCamera();
    const int dX = m_instantZoomFactor * 100;
    m_view->StartZoomAtPoint(currPos.x(), currPos.y());
    m_view->ZoomAtPoint(currPos.x(), currPos.y(), currPos.x() + dX, currPos.y());
    this->redrawView();
}

void V3dViewController::stopInstantZoom()
{
    this->stopDynamicAction();
    this->restoreCamera();
    this->redrawView();
}

void V3dViewController::drawRubberBand(const QPoint& posMin, const QPoint& posMax)
{
    if (!m_rubberBand)
        m_rubberBand = this->createRubberBand();

    QRect rect;
    rect.setX(std::min(posMin.x(), posMax.x()));
    rect.setY(std::min(posMin.y(), posMax.y()));
    rect.setWidth(std::abs(posMax.x() - posMin.x()));
    rect.setHeight(std::abs(posMax.y() - posMin.y()));
    m_rubberBand->updateGeometry(rect);
    m_rubberBand->setVisible(true);
}

void V3dViewController::hideRubberBand()
{
    if (m_rubberBand)
        m_rubberBand->setVisible(false);
}

void V3dViewController::backupCamera()
{
    if (!m_cameraBackup)
        m_cameraBackup = new Graphic3d_Camera;

    m_cameraBackup->Copy(m_view->Camera());
}

void V3dViewController::restoreCamera()
{
    if (m_cameraBackup)
        m_view->Camera()->Copy(m_cameraBackup);
}

void V3dViewController::redrawView()
{
    m_view->Redraw();
}

V3dViewController::DynamicAction V3dViewController::currentDynamicAction() const
{
    return m_dynamicAction;
}

bool V3dViewController::hasCurrentDynamicAction() const
{
    return m_dynamicAction != DynamicAction::None;
}

} // namespace Mayo
