/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtGui/QColor>
class QComboBox;

namespace Mayo {

struct Theme {
    enum class Color {
        FlatBackground,
        FlatChecked,
        FlatHover,
        ButtonView3dBackground,
        ButtonView3dChecked,
        ButtonView3dHover
    };
    enum class Image {
        FlatDownIndicator,
        FlatDownIndicatorDisabled
    };
    QColor color(Color role) const;
    QString imageUrl(Image img) const;

    void makeFlat(QComboBox* comboBox);

private:
    Theme() = default;
    friend Theme* mayoTheme();
};

Theme* mayoTheme();

} // namespace Mayo
