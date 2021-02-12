/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../graphics/v3d_view_controller.h"

class QCursor;
class QRubberBand;

namespace Mayo {

class WidgetOccView;

class WidgetOccViewController : public V3dViewController {
    Q_OBJECT
public:
    WidgetOccViewController(WidgetOccView* widgetView = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void multiSelectionToggled(bool on);

private:
    void setViewCursor(const QCursor& cursor);

    AbstractRubberBand* createRubberBand() override;
    struct RubberBand;

    WidgetOccView* m_widgetView = nullptr;
    QPoint m_prevPos;
    QPoint m_posRubberBandStart;
};

} // namespace Mayo
