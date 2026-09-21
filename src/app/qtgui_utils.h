/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/tkernel_utils.h"

#include <Quantity_Color.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Quantity_NameOfColor.hxx>
#include <Image_PixMap.hxx>

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QGradient>
#include <QtGui/QPixmap>

#include <type_traits>

class QImage;
class QMouseEvent;
class QWheelEvent;
class QScreen;

// Provides a collection of tools for the QtGui module
namespace Mayo::QtGuiUtils {

// Color conversion functions OCCT -> Qt
QColor toQColor(const Quantity_Color& c);
QColor toQColor(const Quantity_ColorRGBA& c);
QColor toQColor(Quantity_NameOfColor c);

Quantity_Color toOccColor(const QColor& c, Quantity_TypeOfColor colorType);
Quantity_ColorRGBA toOccColorRGBA(const QColor& c, Quantity_TypeOfColor colorType);
Quantity_NameOfColor toOccColorName(const QColor& c, Quantity_TypeOfColor colorType);

Quantity_Color toPreferredColorSpace(const QColor& c);

// Converts (OCCT)Image_PixMap -> QPixmap
QPixmap toQPixmap(const Image_PixMap& pixmap);

// Loads QPixmap from a QByteArray object
// The loader probes the data in 'bytes' for a header to guess the file format
QPixmap toQPixmap(const QByteArray& bytes, Qt::ImageConversionFlags flags = Qt::AutoColor);

// Saves QPixmap into a QByteArray object
QByteArray toQByteArray(const QPixmap& pixmap, const char* format = "PNG");

// Converts QImage -> (OCCT)Image_PixMap
// The image is converted to 32-bit RGBA format before being copied into the destination pixmap.
// The destination owns its pixel data after this call, so it remains valid independently of the
// source QImage
// Returns TRUE if the conversion succeeds, FALSE otherwise
bool toOccPixmap(const QImage& image, Image_PixMap& occPixmap);

// Returns linear interpolated color between 'a' and 'b' at parameter 't'
QColor lerp(const QColor& a, const QColor& b, double t);

// Returns color at normalized pos 't' from linear gradient
// Note: assumes that gradient stops array contains keys 0 and 1
QColor linearColorAt(const QGradient& gradient, double t);

// Returns version of 'gradient' having stop keys 0 and 1
QGradient gradientEndsAdded(const QGradient& gradient);

QGradient subGradient(const QGradient& gradient, double t1, double t2);

// Returns corresponding width in pixels proportional to 'screen' width resolution
// 'screenRatio' must be within [0,1]
int screenPixelWidth(double screenRatio, const QScreen* screen = nullptr);
int screenPixelHeight(double screenRatio, const QScreen* screen = nullptr);
QSize screenPixelSize(double widthRatio, double heightRatio, const QScreen* screen = nullptr);

// Returns the global position of the mouse cursor at the time of the event
// This is a helper function to facilitates Qt5/Qt6 portability as Qt5 QMouseEvent::globalPos() has
// been deprecated in Qt6
QPoint globalPosition(const QMouseEvent* event);
QPoint globalPosition(const QWheelEvent* event);

// Fluent-like helper to change font properties
class FontChange {
public:
    explicit FontChange(const QFont& font);

    FontChange& size(int size);
    FontChange& adjustSize(int offset);
    FontChange& scalePointSizeF(double f);
    FontChange& bold(bool on = true);
    FontChange& fixedPitch(bool on = true);

    constexpr operator const QFont&() const { return m_font; }

private:
    QFont m_font;
};

} // namespace Mayo::QtGuiUtils
