/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "v3d_view_camera_animation.h"

#include "../base/unit_system.h"

namespace Mayo {

V3dViewCameraAnimation::V3dViewCameraAnimation()
    : m_cameraStart(new Graphic3d_Camera),
      m_cameraEnd(new Graphic3d_Camera)
{
}

void V3dViewCameraAnimation::setBackend(std::unique_ptr<IAnimationBackend> anim)
{
    m_backend = std::move(anim);
    if (m_backend) {
        m_backend->setDuration(m_duration);
        m_backend->setTimerCallback([=](QuantityTime t){ this->updateCurrentTime(t); });
    }
}

void V3dViewCameraAnimation::setView(const OccHandle<V3d_View>& view)
{
    if (this->isRunning())
        this->stop();

    m_view = view;
}

void V3dViewCameraAnimation::setDuration(QuantityTime t)
{
    m_duration = t;
    if (m_backend)
        m_backend->setDuration(t);
}

bool V3dViewCameraAnimation::isRunning() const
{
    return m_backend ? m_backend->isRunning() : false;
}

void V3dViewCameraAnimation::start()
{
    if (m_backend)
        m_backend->start();
}

void V3dViewCameraAnimation::stop()
{
    if (m_backend)
        m_backend->stop();
}

void V3dViewCameraAnimation::setCameraStart(const OccHandle<Graphic3d_Camera>& camera)
{
    m_cameraStart->Copy(camera);
}

void V3dViewCameraAnimation::setCameraEnd(const OccHandle<Graphic3d_Camera>& camera)
{
    m_cameraEnd->Copy(camera);
}

void V3dViewCameraAnimation::configureCameraChange(const ViewFunction& fnViewChange)
{
    if (!m_view)
        return;

    if (this->isRunning())
        this->stop();

    const bool wasImmediateUpdateOn = m_view->SetImmediateUpdate(false);
    m_cameraStart->Copy(m_view->Camera());
    fnViewChange(m_view);
    m_cameraEnd->Copy(m_view->Camera());
    m_view->Camera()->Copy(m_cameraStart); // Restore
    m_view->SetImmediateUpdate(wasImmediateUpdateOn);
}

void V3dViewCameraAnimation::setRenderFunction(ViewFunction fnViewRender)
{
    m_fnViewRender = std::move(fnViewRender);
}

void V3dViewCameraAnimation::updateCurrentTime(QuantityTime currTime)
{
    if (!m_view)
        return;

    const double t = m_backend ? m_backend->valueForProgress(currTime / m_duration) : 0.;
    const bool prevImmediateUpdate = m_view->SetImmediateUpdate(false);
    const Graphic3d_CameraLerp cameraLerp(m_cameraStart, m_cameraEnd);
    OccHandle<Graphic3d_Camera> camera = m_view->Camera();
    cameraLerp.Interpolate(t, camera);
    m_view->SetCamera(camera);
    m_view->ZFitAll();
    m_view->SetImmediateUpdate(prevImmediateUpdate);
    if (m_fnViewRender)
        m_fnViewRender(m_view);
    else
        m_view->Update();
}

} // namespace Mayo
