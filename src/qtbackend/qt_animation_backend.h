/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../gui/v3d_view_camera_animation.h"

#include <QtCore/QAbstractAnimation>
#include <QtCore/QEasingCurve>

namespace Mayo {

// Provides implementation of IAnimationBackend based on QAbstractAnimation
class QtAnimationBackend : public IAnimationBackend {
public:
    QtAnimationBackend(QEasingCurve::Type easingType = QEasingCurve::Linear);

    void setDuration(QuantityTime t) override;

    bool isRunning() const override;

    void start() override;
    void stop() override;

    double valueForProgress(double p) const override;

    void setTimerCallback(std::function<void(QuantityTime)> fn) override;

private:
    class AnimationImpl : public QAbstractAnimation {
    public:
        double m_duration_ms = 1000.;
        std::function<void(QuantityTime)> m_callback;

        int duration() const override;

    protected:
        void updateCurrentTime(int currentTime) override;
    };

    AnimationImpl m_impl;
    QEasingCurve m_easingCurve;
};

} // namespace Mayo
