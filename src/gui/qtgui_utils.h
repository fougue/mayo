/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtGui/QColor>
#include <QtGui/QGradient>

namespace Mayo {

class QtGuiUtils {
public:
    // Returns linear interpolated color between 'a' and 'b' at parameter 't'
    static QColor lerp(const QColor& a, const QColor& b, double t);

    // Returns color at normalized pos 't' from linear gradient
    // Note: assumes that gradient stops array contains keys 0 and 1
    static QColor linearColorAt(const QGradient& gradient, double t);

    // Returns version of 'gradient' having stop keys 0 and 1
    static QGradient gradientEndsAdded(const QGradient& gradient);

    static QGradient subGradient(const QGradient& gradient, double t1, double t2);
};

} // namespace Mayo
