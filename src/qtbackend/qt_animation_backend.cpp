/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qt_animation_backend.h"

#include "../base/unit_system.h"

namespace Mayo {

QtAnimationBackend::QtAnimationBackend(QEasingCurve::Type easingType)
    : m_easingCurve(easingType)
{
}

void QtAnimationBackend::setDuration(QuantityTime t)
{
    m_impl.m_duration_ms = UnitSystem::milliseconds(t);
}

bool QtAnimationBackend::isRunning() const
{
    return m_impl.state() == QAbstractAnimation::Running;
}

void QtAnimationBackend::start()
{
    m_impl.start(QAbstractAnimation::KeepWhenStopped);
}

void QtAnimationBackend::stop()
{
    m_impl.stop();
}

double QtAnimationBackend::valueForProgress(double p) const
{
    return m_easingCurve.valueForProgress(p);
}

void QtAnimationBackend::setTimerCallback(std::function<void(QuantityTime)> fn)
{
    m_impl.m_callback = std::move(fn);
}

int QtAnimationBackend::AnimationImpl::duration() const
{
    return static_cast<int>(m_duration_ms);
}

void QtAnimationBackend::AnimationImpl::updateCurrentTime(int currentTime)
{
    m_callback(currentTime * Quantity_Millisecond);
}

} // namespace Mayo
