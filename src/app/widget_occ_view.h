/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"

#include <Graphic3d_GraphicDriver.hxx>
#include <Standard_Version.hxx>
#include <V3d_View.hxx>

#include <QtWidgets/QWidget>
#if OCC_VERSION_HEX >= 0x070600
#  include <QOpenGLWidget> // WARNING Qt5 <QtWidgets/...> / Qt6 <QtOpenGLWidgets/...>
#endif

#include <functional>

namespace Mayo {

// Base interface for bridging Qt and OpenCascade 3D view
// IWidgetOccView does not handle input devices interaction like keyboard and mouse
class IWidgetOccView {
public:
    const OccHandle<V3d_View>& v3dView() const { return m_view; }

    virtual void redraw() = 0;
    virtual QWidget* widget() = 0;
    virtual bool supportsWidgetOpacity() const = 0;

    using Creator = std::function<IWidgetOccView* (const OccHandle<V3d_View>&, QWidget*)>;
    static void setCreator(Creator fn);
    static IWidgetOccView* create(const OccHandle<V3d_View>& view, QWidget* parent = nullptr);

protected:
    IWidgetOccView(const OccHandle<V3d_View>& view) : m_view(view) {}

private:
    OccHandle<V3d_View> m_view;
};

#if OCC_VERSION_HEX >= 0x070600
// Integration of OpenCascade 7.6 with QOpenGLWidget allows widgets with translucid background to be
// correctly displayed over V3d_View
// QOpenGLWidgetOccView implementation is based on https://github.com/gkv311/occt-samples-qopenglwidget
class QOpenGLWidgetOccView : public QOpenGLWidget, public IWidgetOccView {
public:
    QOpenGLWidgetOccView(const OccHandle<V3d_View>& view, QWidget* parent = nullptr);

    void redraw() override;
    QWidget* widget() override { return this; }
    bool supportsWidgetOpacity() const override { return true; }

    static QOpenGLWidgetOccView* create(const OccHandle<V3d_View>& view, QWidget* parent);
    static OccHandle<Graphic3d_GraphicDriver> createCompatibleGraphicsDriver();

protected:
    // -- QOpenGLWidget
    void initializeGL() override;
    void paintGL() override;
};
#endif

// Fallback using traditional QWidget wrapper, no translucid background support
class QWidgetOccView : public QWidget, public IWidgetOccView {
public:
    QWidgetOccView(const OccHandle<V3d_View>& view, QWidget* parent = nullptr);

    void redraw() override;
    QWidget* widget() override { return this; }
    bool supportsWidgetOpacity() const override { return false; }

    static QWidgetOccView* create(const OccHandle<V3d_View>& view, QWidget* parent);
    static OccHandle<Graphic3d_GraphicDriver> createCompatibleGraphicsDriver();

protected:
    // -- QWidget
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    QPaintEngine* paintEngine() const override { return nullptr; }
};

} // namespace Mayo
