/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
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
        Graphic3d_AspectFillArea,
        View3d_BackgroundGradientStart,
        View3d_BackgroundGradientEnd,
        RubberBandView3d_Line,
        RubberBandView3d_Fill,
        MessageIndicator_InfoBackground,
        MessageIndicator_InfoText,
        MessageIndicator_ErrorBackground,
        MessageIndicator_ErrorText
    };

    enum class Icon {
        AddFile,
        File,
        OpenFiles,
        Import,
        Edit,
        Export,
        Expand,
        Cross,
        Link,
        Back, Next,
        Multiple,
        Camera,
        LeftSidebar,
        BackSquare,
        IndicatorDown,
        Reload,
        Stop,
        Gear,
        ZoomIn, ZoomOut,
        Grid, ClipPlane, Measure,
        View3dIso, View3dLeft, View3dRight, View3dTop, View3dBottom, View3dFront, View3dBack,
        VisibilityMenu, VisibilityShowAll, VisibilityShowSelection, VisibilityHideSelection, VisibilityShowSelectionOnly,
        TurnClockwise, TurnCounterClockwise,
        //
        ItemMesh, ItemXde,
        //
        XdeAssembly, XdeSimpleShape
    };

    virtual ~Theme() = default;

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
