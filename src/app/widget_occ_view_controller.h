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

class IWidgetOccView;

class WidgetOccViewController : public V3dViewController {
    Q_OBJECT
public:
    WidgetOccViewController(IWidgetOccView* occView = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void multiSelectionToggled(bool on);

protected:
    void redrawView() override;

private:
    void startDynamicAction(DynamicAction action) override;
    void stopDynamicAction() override;

    void setViewCursor(const QCursor& cursor);

    AbstractRubberBand* createRubberBand() override;
    struct RubberBand;

    bool handleEvent(QEvent* event);

    IWidgetOccView* m_occView = nullptr;
    QPoint m_prevPos;
};

} // namespace Mayo
