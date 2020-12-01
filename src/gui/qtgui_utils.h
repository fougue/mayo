/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QGradient>

namespace Mayo {
namespace QtGuiUtils {

// Returns linear interpolated color between 'a' and 'b' at parameter 't'
QColor lerp(const QColor& a, const QColor& b, double t);

// Returns color at normalized pos 't' from linear gradient
// Note: assumes that gradient stops array contains keys 0 and 1
QColor linearColorAt(const QGradient& gradient, double t);

// Returns version of 'gradient' having stop keys 0 and 1
QGradient gradientEndsAdded(const QGradient& gradient);

QGradient subGradient(const QGradient& gradient, double t1, double t2);

class FontChange {
public:
    FontChange(const QFont& font);

    FontChange& size(int size);
    FontChange& adjustSize(int offset);
    FontChange& bold(bool on);
    FontChange& fixedPitch(bool on);

    constexpr operator const QFont&() const { return m_font; }

private:
    QFont m_font;
};

} // namespace QtGuiUtils
} // namespace Mayo
