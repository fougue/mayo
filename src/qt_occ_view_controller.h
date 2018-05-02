/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

signals:
    void viewRotationStarted();
    void viewRotationEnded();
    void viewPanningStarted();
    void viewPanningEnded();
    void viewScaled();
    void mouseMoved(const QPoint& posMouseInView);

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
