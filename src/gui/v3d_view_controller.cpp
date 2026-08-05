/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "v3d_view_controller.h"
#include "../base/unit_system.h"

#include <AIS_ViewController.hxx>
#include <Graphic3d_Camera.hxx>
#include <V3d_View.hxx>

#include <algorithm>
#include <cmath>

namespace Mayo {

namespace {

class AisViewControllerImpl : public AIS_ViewController {
public:
    void zoomAt(const OccHandle<V3d_View>& view, int xPos, int yPos, double delta)
    {
        const Aspect_ScrollDelta scrollInfo(
            NCollection_Vec2<int>{xPos, yPos}, delta * this->myScrollZoomRatio
        );
        AIS_ViewController::handleZoom(view, scrollInfo, nullptr);
        // with AIS_InteractiveContext the tool will be able to pick 3D point to zoom at within perspective camera
        //AIS_ViewController::UpdateMouseScroll(scrollInfo);
        //AIS_ViewController::FlushViewEvents(m_aisCtx, m_view, true);
    }
};

} // namespace

class V3dViewController::Private {
public:
    OccHandle<V3d_View> m_view;
    DynamicAction m_dynamicAction = DynamicAction::None;
    std::unique_ptr<IRubberBand> m_rubberBand;
    double m_instantZoomFactor = 5.;
    OccHandle<Graphic3d_Camera> m_cameraBackup;
    Position m_posRubberBandStart = {};
    AisViewControllerImpl m_aisViewCtrl;
};

V3dViewController::V3dViewController(const OccHandle<V3d_View>& view)
    : d(new Private)
{
    d->m_view = view;
}

V3dViewController::~V3dViewController()
{
    delete d;
}

void V3dViewController::zoomAt([[maybe_unused]]const Position& currPos, double delta)
{
    d->m_aisViewCtrl.zoomAt(d->m_view, currPos.x, currPos.y, delta);
    this->redrawView();
    this->signalViewScaled.send();
}

void V3dViewController::zoomIn()
{
    d->m_view->SetScale(d->m_view->Scale() * 1.1); // +10%
    this->redrawView();
    this->signalViewScaled.send();
}

void V3dViewController::zoomOut()
{
    d->m_view->SetScale(d->m_view->Scale() / 1.1); // -10%
    this->redrawView();
    this->signalViewScaled.send();
}

void V3dViewController::turn(V3d_TypeOfAxe axis, QuantityAngle angle)
{
    d->m_view->Turn(axis, UnitSystem::radians(angle), true/*start*/);
    this->redrawView();
}

double V3dViewController::instantZoomFactor() const
{
    return d->m_instantZoomFactor;
}

void V3dViewController::setInstantZoomFactor(double factor)
{
    d->m_instantZoomFactor = factor;
}

void V3dViewController::startDynamicAction(DynamicAction dynAction)
{
    if (dynAction == DynamicAction::None)
        return;

    if (d->m_dynamicAction != DynamicAction::None)
        return;

    d->m_dynamicAction = dynAction;
    this->signalDynamicActionStarted.send(dynAction);
}

void V3dViewController::stopDynamicAction()
{
    if (d->m_dynamicAction != DynamicAction::None) {
        this->signalDynamicActionEnded.send(d->m_dynamicAction);
        d->m_dynamicAction = DynamicAction::None;
    }
}

bool V3dViewController::isRotationStarted() const
{
    return d->m_dynamicAction == DynamicAction::Rotation;
}

bool V3dViewController::isPanningStarted() const
{
    return d->m_dynamicAction == DynamicAction::Panning;
}

bool V3dViewController::isZoomStarted() const
{
    return d->m_dynamicAction == DynamicAction::Zoom;
}

bool V3dViewController::isWindowZoomingStarted() const
{
    return d->m_dynamicAction == DynamicAction::WindowZoom;
}

void V3dViewController::rotation(const Position& currPos)
{
    if (this->currentDynamicAction() != DynamicAction::Rotation)
        this->stopDynamicAction();

    if (!this->isRotationStarted()) {
        this->startDynamicAction(DynamicAction::Rotation);
        d->m_view->StartRotation(currPos.x, currPos.y);
    }
    else {
        d->m_view->Rotation(currPos.x, currPos.y);
        this->redrawView();
    }
}

void V3dViewController::pan(const Position& prevPos, const Position& currPos)
{
    if (this->currentDynamicAction() != DynamicAction::Panning)
        this->stopDynamicAction();

    if (!this->isPanningStarted())
        this->startDynamicAction(DynamicAction::Panning);

    d->m_view->Pan(currPos.x - prevPos.x, prevPos.y - currPos.y);
    this->redrawView();
}

void V3dViewController::zoom(const Position& prevPos, const Position& currPos)
{
    if (this->currentDynamicAction() != DynamicAction::Zoom)
        this->stopDynamicAction();

    if (!this->isZoomStarted()) {
        this->startDynamicAction(DynamicAction::Zoom);
        d->m_view->StartZoomAtPoint(currPos.x, currPos.y);
    }
    else {
        d->m_view->Zoom(-prevPos.y, 0, -currPos.y, 0); // Zoom by vertical movement
        this->redrawView();
    }
}

void V3dViewController::windowFitAll(const Position& posMin, const Position& posMax)
{
    if (std::abs(posMin.x - posMax.x) > 1 || std::abs(posMin.y - posMax.y) > 1)
        d->m_view->WindowFitAll(posMin.x, posMin.y, posMax.x, posMax.y);
}

void V3dViewController::windowZoomRubberBand(const Position& currPos)
{
    if (!this->isWindowZoomingStarted()) {
        this->startDynamicAction(DynamicAction::WindowZoom);
        d->m_posRubberBandStart = currPos;
    }

    this->drawRubberBand(d->m_posRubberBandStart, currPos);
}

void V3dViewController::windowZoom(const Position& currPos)
{
    this->windowFitAll(d->m_posRubberBandStart, currPos);
    this->hideRubberBand();
}

void V3dViewController::startInstantZoom(const Position& currPos)
{
    this->startDynamicAction(DynamicAction::InstantZoom);
    this->backupCamera();
    const int dX = std::lround(d->m_instantZoomFactor * 100);
    d->m_view->StartZoomAtPoint(currPos.x, currPos.y);
    d->m_view->ZoomAtPoint(currPos.x, currPos.y, currPos.x + dX, currPos.y);
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
    if (!d->m_rubberBand)
        d->m_rubberBand = this->createRubberBand();

    const int xRect = std::min(posMin.x, posMax.x);
    const int yRect = std::min(posMin.y, posMax.y);
    const int width = std::abs(posMax.x - posMin.x);
    const int height = std::abs(posMax.y - posMin.y);
    d->m_rubberBand->updateGeometry(xRect, yRect, width, height);
    d->m_rubberBand->setVisible(true);
}

void V3dViewController::hideRubberBand()
{
    if (d->m_rubberBand)
        d->m_rubberBand->setVisible(false);
}

void V3dViewController::backupCamera()
{
    if (!d->m_cameraBackup)
        d->m_cameraBackup = new Graphic3d_Camera;

    d->m_cameraBackup->Copy(d->m_view->Camera());
}

void V3dViewController::restoreCamera()
{
    if (d->m_cameraBackup)
        d->m_view->Camera()->Copy(d->m_cameraBackup);
}

void V3dViewController::redrawView()
{
    d->m_view->Redraw();
}

V3dViewController::DynamicAction V3dViewController::currentDynamicAction() const
{
    return d->m_dynamicAction;
}

bool V3dViewController::hasCurrentDynamicAction() const
{
    return d->m_dynamicAction != DynamicAction::None;
}

} // namespace Mayo
