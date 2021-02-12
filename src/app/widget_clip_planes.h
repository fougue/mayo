/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QWidget>
#include <Bnd_Box.hxx>
#include <Graphic3d_ClipPlane.hxx>
#include <Graphic3d_TextureMap.hxx>
#include <V3d_View.hxx>
#include <vector>
class QCheckBox;
class QDoubleSpinBox;
class QAbstractSlider;
class QAbstractButton;

namespace Mayo {

class WidgetClipPlanes : public QWidget {
    Q_OBJECT
public:
    WidgetClipPlanes(const Handle_V3d_View& view3d, QWidget* parent = nullptr);
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
        Handle_Graphic3d_ClipPlane graphics;
        UiClipPlane ui;
    };

    using Range = std::pair<double, double>;

    void connectUi(ClipPlaneData* data);

    void setPlaneOn(const Handle_Graphic3d_ClipPlane& plane, bool on);
    void setPlaneRange(ClipPlaneData* data, const Range& range);

    void createPlaneCappingTexture();

    class Ui_WidgetClipPlanes* m_ui;
    Handle_V3d_View m_view;
    std::vector<ClipPlaneData> m_vecClipPlaneData;
    Bnd_Box m_bndBox;
    Handle_Graphic3d_TextureMap m_textureCapping;
};

} // namespace Mayo
