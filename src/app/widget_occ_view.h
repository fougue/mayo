/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>
#include <V3d_View.hxx>

#if OCC_VERSION_HEX >= 0x070600
#  include <QtWidgets/QOpenGLWidget>
using MayoWidgetOccView_ParentType = QOpenGLWidget;
#else
#  include <QtWidgets/QWidget>
using MayoWidgetOccView_ParentType = QWidget;
#endif

namespace Mayo {

// Qt wrapper around the V3d_View class
// WidgetOccView does not handle input devices interaction like keyboard and mouse
//
// Integration of OpenCascade 7.6 with QOpenGLWidget allows widgets with translucid background to be
// correctly displayed over V3d_View
// WidgetOccView implementation is based on https://github.com/gkv311/occt-samples-qopenglwidget
class WidgetOccView : public MayoWidgetOccView_ParentType {
    Q_OBJECT
public:
    WidgetOccView(const Handle_V3d_View& view, QWidget* parent = nullptr);

    const Handle_V3d_View& v3dView() const { return m_view; }

    void redraw();

protected:
#if OCC_VERSION_HEX >= 0x070600
    // -- QOpenGLWidget
    void initializeGL() override;
    void paintGL() override;
#else
    // -- QWidget
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    QPaintEngine* paintEngine() const override { return nullptr; }
#endif

private:
    Handle_V3d_View m_view;
};

} // namespace Mayo
