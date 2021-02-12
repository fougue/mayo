/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QWidget>
#include <V3d_View.hxx>

namespace Mayo {

//! Qt wrapper around the V3d_View class
//! WidgetOccView does not handle input devices interaction like keyboard and mouse
class WidgetOccView : public QWidget {
    Q_OBJECT
public:
    WidgetOccView(const Handle_V3d_View& view, QWidget* parent = nullptr);

    const Handle_V3d_View& v3dView() const;

    QPaintEngine* paintEngine() const override;

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    Handle_V3d_View m_view;
};

} // namespace Mayo
