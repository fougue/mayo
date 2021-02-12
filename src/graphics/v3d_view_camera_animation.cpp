/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "v3d_view_camera_animation.h"

namespace Mayo {

V3dViewCameraAnimation::V3dViewCameraAnimation(const Handle_V3d_View& view, QObject* parent)
    : QAbstractAnimation(parent),
      m_view(view),
      m_cameraStart(new Graphic3d_Camera),
      m_cameraEnd(new Graphic3d_Camera)
{
}

int V3dViewCameraAnimation::duration() const
{
    return m_duration_ms;
}

void V3dViewCameraAnimation::setDuration(int msecs)
{
    m_duration_ms = msecs;
}

void V3dViewCameraAnimation::setCameraStart(const Handle_Graphic3d_Camera& camera)
{
    m_cameraStart->Copy(camera);
}

void V3dViewCameraAnimation::setCameraEnd(const Handle_Graphic3d_Camera& camera)
{
    m_cameraEnd->Copy(camera);
}

const QEasingCurve& V3dViewCameraAnimation::easingCurve() const
{
    return m_easingCurve;
}

void V3dViewCameraAnimation::setEasingCurve(const QEasingCurve& easingCurve)
{
    m_easingCurve = easingCurve;
}

void V3dViewCameraAnimation::configure(const std::function<void(Handle_V3d_View)>& fnViewChange)
{
    if (this->state() == QAbstractAnimation::Running)
        this->stop();

    const bool wasImmediateUpdateOn = m_view->SetImmediateUpdate(false);
    m_cameraStart->Copy(m_view->Camera());
    fnViewChange(m_view);
    m_cameraEnd->Copy(m_view->Camera());
    m_view->Camera()->Copy(m_cameraStart); // Restore
    m_view->SetImmediateUpdate(wasImmediateUpdateOn);
}

void V3dViewCameraAnimation::updateCurrentTime(int currentTime)
{
    const double t = m_easingCurve.valueForProgress(currentTime / double(m_duration_ms));
    const bool prevImmediateUpdate = m_view->SetImmediateUpdate(false);
    const Graphic3d_CameraLerp cameraLerp(m_cameraStart, m_cameraEnd);
    Handle_Graphic3d_Camera camera = m_view->Camera();
    cameraLerp.Interpolate(t, camera);
    m_view->SetCamera(camera);
    m_view->ZFitAll();
    m_view->SetImmediateUpdate(prevImmediateUpdate);
    m_view->Update();
}

} // namespace Mayo
