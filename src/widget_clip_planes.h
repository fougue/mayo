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

#include <QtWidgets/QWidget>
#include <Bnd_Box.hxx>
#include <Graphic3d_ClipPlane.hxx>
#include <vector>
class QCheckBox;
class QDoubleSpinBox;
class QAbstractSlider;
class QAbstractButton;

namespace Mayo {

class WidgetOccView;

class WidgetClipPlanes : public QWidget {
    Q_OBJECT

public:
    WidgetClipPlanes(WidgetOccView* view, QWidget* parent = nullptr);
    ~WidgetClipPlanes();

    void setRanges(const Bnd_Box& box);
    void setClippingOn(bool on);

private:
    struct UiClipPlane {
        QCheckBox* check_On;
        QWidget* widget_Control;

        UiClipPlane(QCheckBox* checkOn, QWidget* widgetControl);
        QDoubleSpinBox* posSpin() const;
        QAbstractSlider* posSlider() const;
        QAbstractButton* inverseBtn() const;
        QDoubleSpinBox* customXDirSpin() const;
        QDoubleSpinBox* customYDirSpin() const;
        QDoubleSpinBox* customZDirSpin() const;
        double spinValueToSliderValue(double val) const;
        double sliderValueToSpinValue(double val) const;
    };

    struct ClipPlaneData {
        Handle_Graphic3d_ClipPlane gpx;
        UiClipPlane ui;
    };

    using Range = std::pair<double, double>;

    void connectUi(ClipPlaneData* data);

    void setPlaneOn(const Handle_Graphic3d_ClipPlane& plane, bool on);
    void setPlaneRange(ClipPlaneData* data, const Range& range);

    class Ui_WidgetClipPlanes* m_ui;
    WidgetOccView* m_view;
    std::vector<ClipPlaneData> m_vecClipPlaneData;
    Bnd_Box m_bndBox;
};

} // namespace Mayo
