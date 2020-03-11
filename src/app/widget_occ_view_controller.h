/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../gpx/v3d_view_controller.h"

#include <Graphic3d_Camera.hxx>
class QCursor;
class QRubberBand;

namespace Mayo {

class WidgetOccView;

class WidgetOccViewController : public V3dViewController {
    Q_OBJECT
public:
    WidgetOccViewController(WidgetOccView* widgetView = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setViewCursor(const QCursor& cursor);

    AbstractRubberBand* createRubberBand() override;
    struct RubberBand;

    WidgetOccView* m_widgetView = nullptr;
    QPoint m_prevPos;
    QPoint m_posRubberBandStart;
    Handle_Graphic3d_Camera m_prevCamera;
};

} // namespace Mayo
