/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
class QComboBox;

namespace Mayo {

class Theme {
public:
    enum class Color {
        Palette_Base,
        Palette_Window,
        Palette_Button,
        ButtonFlat_Background,
        ButtonFlat_Hover,
        ButtonFlat_Checked,
        ButtonView3d_Background,
        ButtonView3d_Hover,
        ButtonView3d_Checked,
        View3d_BackgroundGradientStart,
        View3d_BackgroundGradientEnd,
        MessageIndicator_Background,
        MessageIndicator_Text
    };

    enum class Icon {
        Expand,
        Cross,
        Link,
        Pin,
        Back,
        Next,
        Camera,
        LeftSidebar,
        LeftArrowCross,
        IndicatorDown,
        Stop,
        Gear,
        ZoomIn,
        ZoomOut,
        ItemMesh
    };

    virtual const QIcon& icon(Icon icn) const = 0;
    virtual QColor color(Color role) const = 0;

    virtual void setup() = 0;
    virtual void setupHeaderComboBox(QComboBox* cb) = 0;

protected:
    Theme() = default;
    friend Theme* mayoTheme();
};

Theme* createTheme(const QString& key);
Theme* mayoTheme();

} // namespace Mayo
