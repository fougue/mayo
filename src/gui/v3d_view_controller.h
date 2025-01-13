/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"
#include "../base/quantity.h"
#include "../base/signal.h"
#include "gui_vkey_mouse.h"

#include <Graphic3d_Camera.hxx>
#include <V3d_View.hxx>
#include <memory>

namespace Mayo {

class V3dViewController {
public:
    enum class DynamicAction {
        None,
        Panning,
        Rotation,
        Zoom,
        WindowZoom,
        InstantZoom
    };

    struct IRubberBand {
        virtual ~IRubberBand() {}
        virtual void updateGeometry(int x, int y, int width, int height) = 0;
        virtual void setVisible(bool on) = 0;
    };

    V3dViewController(const OccHandle<V3d_View>& view);
    virtual ~V3dViewController() = default;

    DynamicAction currentDynamicAction() const;
    bool hasCurrentDynamicAction() const;

    void zoomIn();
    void zoomOut();

    // Rotates the view point around an axis of frame of reference for which the origin is the
    // eye of the projection
    void turn(V3d_TypeOfAxe axis, QuantityAngle angle);

    double instantZoomFactor() const { return m_instantZoomFactor; }
    void setInstantZoomFactor(double factor) { m_instantZoomFactor = factor; }

    // Signals
    Signal<DynamicAction> signalDynamicActionStarted;
    Signal<DynamicAction> signalDynamicActionEnded;
    Signal<> signalViewScaled;
    Signal<int, int> signalMouseMoved; // x,y: mouse position in view
    Signal<Aspect_VKeyMouse> signalMouseButtonClicked;
    Signal<bool> signalMultiSelectionToggled;

protected:
    struct Position { int x; int y; };

    virtual void startDynamicAction(DynamicAction dynAction);
    virtual void stopDynamicAction();

    bool isRotationStarted() const;
    bool isPanningStarted() const;
    bool isZoomStarted() const;
    bool isWindowZoomingStarted() const;

    void rotation(const Position& currPos);
    void pan(const Position& prevPos, const Position& currPos);
    void zoom(const Position& prevPos, const Position& currPos);

    void windowFitAll(const Position& posMin, const Position& posMax);

    void windowZoomRubberBand(const Position& currPos);
    void windowZoom(const Position& currPos);

    void startInstantZoom(const Position& currPos);
    void stopInstantZoom();

    virtual std::unique_ptr<IRubberBand> createRubberBand() = 0;
    void drawRubberBand(const Position& posMin, const Position& posMax);
    void hideRubberBand();

    void backupCamera();
    void restoreCamera();

    virtual void redrawView();

private:
    OccHandle<V3d_View> m_view;
    DynamicAction m_dynamicAction = DynamicAction::None;
    std::unique_ptr<IRubberBand> m_rubberBand;
    double m_instantZoomFactor = 5.;
    OccHandle<Graphic3d_Camera> m_cameraBackup;
    Position m_posRubberBandStart = {};
};

} // namespace Mayo
