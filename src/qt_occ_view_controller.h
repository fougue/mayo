/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <V3d_View.hxx>
#include <QtCore/QObject>
#include <QtCore/QPoint>
class QCursor;

namespace Mayo {

class BaseV3dViewController : public QObject {
    Q_OBJECT
public:
    BaseV3dViewController(const Handle_V3d_View& view, QObject* parent = nullptr);

    bool isRotating() const;
    bool isPanning() const;

    void zoomIn();
    void zoomOut();

signals:
    void viewRotationStarted();
    void viewRotationEnded();
    void viewPanningStarted();
    void viewPanningEnded();
    void viewScaled();
    void mouseMoved(const QPoint& posMouseInView);
    void mouseClicked(Qt::MouseButton btn);

protected:
    void setStateRotation(bool on);
    void setStatePanning(bool on);

private:
    Handle_V3d_View m_view;
    bool m_stateRotation = false;
    bool m_statePanning = false;
};

class WidgetOccView;

class QtOccViewController : public BaseV3dViewController {
    Q_OBJECT
public:
    QtOccViewController(WidgetOccView* widgetView = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setViewCursor(const QCursor& cursor);

    WidgetOccView* m_widgetView = nullptr;
    QPoint m_prevPos;
};

} // namespace Mayo
