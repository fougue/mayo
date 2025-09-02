/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"
#include "../base/quantity.h"

#include <V3d_View.hxx>
#include <functional>
#include <memory>

namespace Mayo {

// Provides interface for animation base mechanism(backend)
class IAnimationBackend {
public:
    virtual ~IAnimationBackend() = default;
    virtual void setDuration(QuantityTime t) = 0;
    virtual bool isRunning() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual double valueForProgress(double p) const = 0; // 'p' in [0,1]
    virtual void setTimerCallback(std::function<void(QuantityTime)> fn) = 0;
};

// Provides animation control for 3D view camera
class V3dViewCameraAnimation {
public:
    using ViewFunction = std::function<void(const OccHandle<V3d_View>&)>;

    V3dViewCameraAnimation();
    ~V3dViewCameraAnimation() = default;

    bool hasBackend() const { return m_backend.get() != nullptr; }
    void setBackend(std::unique_ptr<IAnimationBackend> anim);

    const OccHandle<V3d_View>& view() const { return m_view; }
    void setView(const OccHandle<V3d_View>& view);

    QuantityTime duration() const { return m_duration; }
    void setDuration(QuantityTime t);

    bool isRunning() const;
    void start();
    void stop();

    const OccHandle<Graphic3d_Camera>& cameraStart() const { return m_cameraStart; }
    void setCameraStart(const OccHandle<Graphic3d_Camera>& camera);

    const OccHandle<Graphic3d_Camera>& cameraEnd() const { return m_cameraEnd; }
    void setCameraEnd(const OccHandle<Graphic3d_Camera>& camera);

    void configureCameraChange(const ViewFunction& fnViewChange);

    void setRenderFunction(ViewFunction fnViewRender);

private:
    void updateCurrentTime(QuantityTime currTime);

    std::unique_ptr<IAnimationBackend> m_backend;
    OccHandle<V3d_View> m_view;
    OccHandle<Graphic3d_Camera> m_cameraStart;
    OccHandle<Graphic3d_Camera> m_cameraEnd;
    QuantityTime m_duration = 1 * Quantity_Second;
    std::function<void(const OccHandle<V3d_View>&)> m_fnViewRender;
};

} // namespace Mayo
