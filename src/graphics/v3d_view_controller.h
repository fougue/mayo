/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Graphic3d_Camera.hxx>
#include <V3d_View.hxx>
#include <QtCore/QObject>
#include <QtCore/QPoint>

namespace Mayo {

class V3dViewController : public QObject {
    Q_OBJECT
public:
    enum class DynamicAction {
        None,
        Panning,
        Rotation,
        WindowZoom,
        InstantZoom
    };

    struct AbstractRubberBand {
        virtual ~AbstractRubberBand() {}
        virtual void updateGeometry(const QRect& rect) = 0;
        virtual void setVisible(bool on) = 0;
    };

    V3dViewController(const Handle_V3d_View& view, QObject* parent = nullptr);
    virtual ~V3dViewController();

    DynamicAction currentDynamicAction() const;
    bool hasCurrentDynamicAction() const;

    void zoomIn();
    void zoomOut();

    double instantZoomFactor() const { return m_instantZoomFactor; }
    void setInstantZoomFactor(double factor) { m_instantZoomFactor = factor; }

signals:
    void dynamicActionStarted(DynamicAction dynAction);
    void dynamicActionEnded(DynamicAction dynAction);
    void viewScaled();

    void mouseMoved(const QPoint& posMouseInView);
    void mouseClicked(Qt::MouseButton btn);

protected:
    void startDynamicAction(DynamicAction dynAction);
    void stopDynamicAction();

    bool isRotationStarted() const;
    bool isPanningStarted() const;
    bool isWindowZoomingStarted() const;

    void instantZoomAt(const QPoint& pos);
    void windowFitAll(const QPoint& posMin, const QPoint& posMax);

    virtual AbstractRubberBand* createRubberBand() = 0;
    void drawRubberBand(const QPoint& posMin, const QPoint& posMax);
    void hideRubberBand();

    void backupCamera();
    void restoreCamera();

private:
    Handle_V3d_View m_view;
    DynamicAction m_dynamicAction = DynamicAction::None;
    AbstractRubberBand* m_rubberBand = nullptr;
    double m_instantZoomFactor = 5.;
    Handle_Graphic3d_Camera m_cameraBackup;
};

} // namespace Mayo
