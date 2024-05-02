/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "v3d_view_controller.h"
#include "../base/unit_system.h"

#include <V3d_View.hxx>
#include <algorithm>
#include <cmath>

namespace Mayo {

V3dViewController::V3dViewController(const OccHandle<V3d_View>& view)
    : m_view(view)
{
}

void V3dViewController::zoomIn()
{
    m_view->SetScale(m_view->Scale() * 1.1); // +10%
    this->redrawView();
    this->signalViewScaled.send();
}

void V3dViewController::zoomOut()
{
    m_view->SetScale(m_view->Scale() / 1.1); // -10%
    this->redrawView();
    this->signalViewScaled.send();
}

void V3dViewController::turn(V3d_TypeOfAxe axis, QuantityAngle angle)
{
    m_view->Turn(axis, UnitSystem::radians(angle), true/*start*/);
    this->redrawView();
}

void V3dViewController::startDynamicAction(DynamicAction dynAction)
{
    if (dynAction == DynamicAction::None)
        return;

    if (m_dynamicAction != DynamicAction::None)
        return;

    m_dynamicAction = dynAction;
    this->signalDynamicActionStarted.send(dynAction);
}

void V3dViewController::stopDynamicAction()
{
    if (m_dynamicAction != DynamicAction::None) {
        this->signalDynamicActionEnded.send(m_dynamicAction);
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

bool V3dViewController::isZoomStarted() const
{
    return m_dynamicAction == DynamicAction::Zoom;
}

bool V3dViewController::isWindowZoomingStarted() const
{
    return m_dynamicAction == DynamicAction::WindowZoom;
}

void V3dViewController::rotation(const Position& currPos)
{
    if (this->currentDynamicAction() != DynamicAction::Rotation)
        this->stopDynamicAction();

    if (!this->isRotationStarted()) {
        this->startDynamicAction(DynamicAction::Rotation);
        m_view->StartRotation(currPos.x, currPos.y);
    }
    else {
        m_view->Rotation(currPos.x, currPos.y);
        this->redrawView();
    }
}

void V3dViewController::pan(const Position& prevPos, const Position& currPos)
{
    if (this->currentDynamicAction() != DynamicAction::Panning)
        this->stopDynamicAction();

    if (!this->isPanningStarted())
        this->startDynamicAction(DynamicAction::Panning);

    m_view->Pan(currPos.x - prevPos.x, prevPos.y - currPos.y);
    this->redrawView();
}

void V3dViewController::zoom(const Position& prevPos, const Position& currPos)
{
    if (this->currentDynamicAction() != DynamicAction::Zoom)
        this->stopDynamicAction();

    if (!this->isZoomStarted()) {
        this->startDynamicAction(DynamicAction::Zoom);
        m_view->StartZoomAtPoint(currPos.x, currPos.y);
    }
    else {
        m_view->Zoom(-prevPos.y, 0, -currPos.y, 0); // Zoom by vertical movement
        this->redrawView();
    }
}

void V3dViewController::windowFitAll(const Position& posMin, const Position& posMax)
{
    if (std::abs(posMin.x - posMax.x) > 1 || std::abs(posMin.y - posMax.y) > 1)
        m_view->WindowFitAll(posMin.x, posMin.y, posMax.x, posMax.y);
}

void V3dViewController::windowZoomRubberBand(const Position& currPos)
{
    if (!this->isWindowZoomingStarted()) {
        this->startDynamicAction(DynamicAction::WindowZoom);
        m_posRubberBandStart = currPos;
    }

    this->drawRubberBand(m_posRubberBandStart, currPos);
}

void V3dViewController::windowZoom(const Position& currPos)
{
    this->windowFitAll(m_posRubberBandStart, currPos);
    this->hideRubberBand();
}

void V3dViewController::startInstantZoom(const Position& currPos)
{
    this->startDynamicAction(DynamicAction::InstantZoom);
    this->backupCamera();
    const int dX = std::lround(m_instantZoomFactor * 100);
    m_view->StartZoomAtPoint(currPos.x, currPos.y);
    m_view->ZoomAtPoint(currPos.x, currPos.y, currPos.x + dX, currPos.y);
    this->redrawView();
}

void V3dViewController::stopInstantZoom()
{
    this->stopDynamicAction();
    this->restoreCamera();
    this->redrawView();
}

void V3dViewController::drawRubberBand(const Position& posMin, const Position& posMax)
{
    if (!m_rubberBand)
        m_rubberBand = this->createRubberBand();

    const int xRect = std::min(posMin.x, posMax.x);
    const int yRect = std::min(posMin.y, posMax.y);
    const int width = std::abs(posMax.x - posMin.x);
    const int height = std::abs(posMax.y - posMin.y);
    m_rubberBand->updateGeometry(xRect, yRect, width, height);
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
