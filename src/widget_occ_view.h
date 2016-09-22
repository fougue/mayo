/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include <functional>
#include <QtWidgets/QWidget>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

namespace Mayo {

/*! Qt wrapper around the V3d_View class
 *
 *  WidgetOccView does not handle input devices interaction like keyboard and mouse.
 */
class WidgetOccView : public QWidget
{
    Q_OBJECT

public:
    WidgetOccView(QWidget* parent = nullptr);
    WidgetOccView(const Handle_V3d_Viewer& viewer, QWidget* parent = nullptr);

    const Handle_V3d_Viewer& occV3dViewer() const;
    void setOccV3dViewer(const Handle_V3d_Viewer& viewer);

    const Handle_V3d_View& occV3dView() const;

    QPaintEngine* paintEngine() const override;

    void redraw();
    void fitAll();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initialize();

    Handle_V3d_Viewer m_viewer;
    Handle_V3d_View m_view;
    bool m_isInitialized = false;
    bool m_needsResize = false;
};

} // namespace Mayo
