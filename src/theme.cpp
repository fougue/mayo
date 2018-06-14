/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "theme.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include <QtWidgets/QComboBox>

namespace Mayo {

QColor Theme::color(Color role) const
{
    switch (role) {
    case Color::FlatBackground:
    case Color::ButtonView3dBackground:
        return QGuiApplication::palette().color(QPalette::Button);
    case Color::FlatChecked:
        return QGuiApplication::palette().color(QPalette::Button).darker(125);
    case Color::FlatHover:
        return QGuiApplication::palette().color(QPalette::Button).darker(110);
    case Color::ButtonView3dHover:
    case Color::ButtonView3dChecked:
        return QColor(65, 200, 250);
    }
    return QColor();
}

QString Theme::imageUrl(Theme::Image img) const
{
    switch (img) {
    case Image::FlatDownIndicator:
        return ":/images/down_8.png";
    case Image::FlatDownIndicatorDisabled:
        return ":/images/down_disabled_8.png";
    }
    return QString();
}

void Theme::makeFlat(QComboBox *comboBox)
{
    const QString comboStyleSheet = QString(
            "QComboBox {"
            "    border-style: solid;"
            "    background: %1;"
            "    padding: 2px 15px 2px 10px;"
            "}\n"
            "QComboBox:hover {"
            "    border-style: solid;"
            "    background: %2;"
            "    padding: 2px 15px 2px 10px;"
            "}\n"
            "QComboBox::drop-down {"
            "    subcontrol-origin: padding;"
            "    subcontrol-position: top right;"
            "    width: 15px;"
            "    border-left-width: 0px;"
            "    border-top-right-radius: 3px;"
            "    border-bottom-right-radius: 3px;"
            "}\n"
            "QComboBox::down-arrow { image: url(%3); }\n"
            "QComboBox::down-arrow:disabled { image: url(%4); }\n"
            ).arg(this->color(Theme::Color::FlatBackground).name(),
                  this->color(Theme::Color::FlatHover).name(),
                  this->imageUrl(Theme::Image::FlatDownIndicator),
                  this->imageUrl(Theme::Image::FlatDownIndicatorDisabled));
    comboBox->setStyleSheet(comboStyleSheet);
}

Theme *mayoTheme()
{
    static Theme theme;
    return &theme;
}

} // namespace Mayo
