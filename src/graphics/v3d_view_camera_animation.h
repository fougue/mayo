/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <V3d_View.hxx>
#include <QtCore/QAbstractAnimation>
#include <QtCore/QEasingCurve>
#include <functional>

namespace Mayo {

class V3dViewCameraAnimation : public QAbstractAnimation {
public:
    using ViewFunction = std::function<void(const Handle_V3d_View&)>;

    V3dViewCameraAnimation(const Handle_V3d_View& view, QObject* parent = nullptr);

    int duration() const override;
    void setDuration(int msecs);

    void setCameraStart(const Handle_Graphic3d_Camera& camera);
    void setCameraEnd(const Handle_Graphic3d_Camera& camera);

    const QEasingCurve& easingCurve() const;
    void setEasingCurve(const QEasingCurve& easingCurve);

    void configure(const ViewFunction& fnViewChange);

    void setRenderFunction(ViewFunction fnViewRender);

protected:
    void updateCurrentTime(int currentTime) override;

private:
    Handle_V3d_View m_view;
    Handle_Graphic3d_Camera m_cameraStart;
    Handle_Graphic3d_Camera m_cameraEnd;
    QEasingCurve m_easingCurve; // Linear by default
    int m_duration_ms = 1000;
    std::function<void(const Handle_V3d_View&)> m_fnViewRender;
};

} // namespace Mayo
