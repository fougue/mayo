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

#include "theme.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>

namespace Mayo {

QColor Theme::color(Color role) const
{
    switch (role) {
    case Color::FlatBackground:
    case Color::ButtonView3dBackground:
        return QGuiApplication::palette().color(QPalette::Button);
    case Color::FlatHover:
        return QGuiApplication::palette().color(QPalette::Button).darker(110);
    case Color::ButtonView3dHover:
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

Theme *mayoTheme()
{
    static Theme theme;
    return &theme;
}

} // namespace Mayo
