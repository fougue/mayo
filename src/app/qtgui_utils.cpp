/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qtgui_utils.h"
#include "../base/math_utils.h"

#include <Standard_Version.hxx>

#include <QtCore/QBuffer>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QScreen>
#include <QtGui/QWindow>

#include <gsl/assert>
#include <algorithm>

namespace Mayo::QtGuiUtils {

namespace {

constexpr bool isLessOrEqual(double lhs, double rhs)
{
    return lhs < rhs || qFuzzyCompare(lhs, rhs);
}

// Is 'v' inside ['start', 'end'] ?
constexpr bool isInRange(double v, double start, double end)
{
    return isLessOrEqual(start, v) && isLessOrEqual(v, end);
}

} // namespace

QColor toQColor(const Quantity_Color& c)
{
    return QColor(c.Red() * 255., c.Green() * 255., c.Blue() * 255.);
}

QColor toQColor(const Quantity_ColorRGBA& c)
{
    QColor qc = QtGuiUtils::toQColor(c.GetRGB());
    qc.setAlphaF(c.Alpha());
    return qc;
}

QColor toQColor(const Quantity_NameOfColor c)
{
    return toQColor(Quantity_Color(c));
}

QColor lerp(const QColor& a, const QColor& b, double t)
{
    Expects(isInRange(t, 0, 1));
    return QColor(
        MathUtils::lerp(a.red(), b.red(), t),
        MathUtils::lerp(a.green(), b.green(), t),
        MathUtils::lerp(a.blue(), b.blue(), t),
        MathUtils::lerp(a.alpha(), b.alpha(), t)
    );
}

QColor linearColorAt(const QGradient& gradient, double t)
{
    Expects(gradient.type() == QGradient::LinearGradient);
    Expects(isInRange(t, 0, 1));
    const QGradientStops stops = gradient.stops();
    if (qFuzzyIsNull(t))
        return stops.front().second;

    auto itLower =
            std::lower_bound(
                stops.cbegin(),
                stops.cend(),
                QGradientStop(t, QColor()),
                [](const QGradientStop& lhs, const QGradientStop& rhs) { return lhs.first < rhs.first; }
    );
    if (itLower != stops.cbegin() && itLower != stops.cend()) {
        const int i = (itLower - stops.cbegin()) - 1;
        const double tMapped = MathUtils::mappedValue(
            t, stops.at(i).first, stops.at(i + 1).first, 0, 1
        );
        return lerp(stops.at(i).second, stops.at(i + 1).second, tMapped);
    }

    return !stops.empty() ? stops.back().second : QColor();
}

QGradient gradientEndsAdded(const QGradient& gradient)
{
    QGradient newGradient = gradient;
    const QGradientStops stops = gradient.stops();
    if (!qFuzzyIsNull(stops.front().first))
        newGradient.setColorAt(0., stops.front().second);

    if (!qFuzzyCompare(stops.back().first, 1.))
        newGradient.setColorAt(1., stops.back().second);

    return newGradient;
}

QGradient subGradient(const QGradient& gradient, double t1, double t2)
{
    if (qFuzzyCompare(t1, 0) && qFuzzyCompare(t2, 1))
        return gradient;

    Expects(isLessOrEqual(t1, t2));
    Expects(gradient.type() == QGradient::LinearGradient);
    QGradientStops subStops;
    subStops.push_back({ 0., linearColorAt(gradient, t1) });
    const QGradientStops stops = gradient.stops();
    for (const QGradientStop& stop : stops) {
        const double t = stop.first;
        const QColor& color = stop.second;
        if (t < t1)
            continue;

        if (t > t2)
            break;

        const double tMapped = MathUtils::mappedValue(t, t1, t2, 0, 1);
        subStops.push_back({ tMapped, color });
    }

    subStops.push_back({ 1., linearColorAt(gradient, t2) });
    QLinearGradient subGradient(0, 0, 1, 0);
    subGradient.setStops(subStops);
    return std::move(subGradient);
}

int screenPixelWidth(double screenRatio, const QScreen* screen)
{
    screen = !screen ? QGuiApplication::primaryScreen() : screen;
    const int screenWidth = screen ? screen->geometry().width() : 800;
    return qRound(screenWidth * screenRatio);
}

int screenPixelHeight(double screenRatio, const QScreen* screen)
{
    screen = !screen ? QGuiApplication::primaryScreen() : screen;
    const int screenHeight = screen ? screen->geometry().height() : 600;
    return qRound(screenHeight * screenRatio);
}

QSize screenPixelSize(double widthRatio, double heightRatio, const QScreen* screen)
{
    return QSize(
        QtGuiUtils::screenPixelWidth(widthRatio, screen),
        QtGuiUtils::screenPixelHeight(heightRatio, screen)
    );
}

QPoint globalPosition(const QMouseEvent* event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return event->globalPos();
#else
    return event->globalPosition().toPoint();
#endif
}

FontChange::FontChange(const QFont& font)
    : m_font(font)
{}

FontChange& FontChange::size(int size)
{
    m_font.setPixelSize(size);
    return *this;
}

FontChange& FontChange::adjustSize(int offset)
{
    const int pixelSize = m_font.pixelSize() > 0 ? m_font.pixelSize() : 12;
    m_font.setPixelSize(pixelSize + offset);
    return *this;
}

FontChange& FontChange::scalePointSizeF(double f) {
    const int pointSizeF = m_font.pointSizeF();
    m_font.setPointSizeF(f * pointSizeF);
    return *this;
}

FontChange& FontChange::bold(bool on)
{
    m_font.setBold(on);
    return *this;
}

FontChange& FontChange::fixedPitch(bool on)
{
    m_font.setFixedPitch(on);
    return *this;
}

Quantity_Color toPreferredColorSpace(const QColor& c)
{
    // See https://dev.opencascade.org/content/occt-3d-viewer-becomes-srgb-aware
#if OCC_VERSION_HEX >= 0x070500
    return QtGuiUtils::toColor<Quantity_TOC_sRGB>(c);
#else
    return QtGuiUtils::toColor<Quantity_TOC_RGB>(c);
#endif
}

QPixmap toQPixmap(const Image_PixMap& pixmap)
{
    auto fnToQImageFormat = [](Image_Format occFormat) {
        switch (occFormat) {
        case Image_Format_RGB:   return QImage::Format_RGB888;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        case Image_Format_BGR:   return QImage::Format_BGR888;
#endif
        case Image_Format_RGBA:  return QImage::Format_ARGB32;
        case Image_Format_RGBF:  return QImage::Format_RGB444;
        case Image_Format_Gray:  return QImage::Format_Grayscale8;
        case Image_Format_GrayF: return QImage::Format_Invalid;
        default: return QImage::Format_Invalid;
        }
    };
    const QImage img(
        pixmap.Data(),
        int(pixmap.Width()),
        int(pixmap.Height()),
        int(pixmap.SizeRowBytes()),
        fnToQImageFormat(pixmap.Format())
    );
    if (img.isNull())
        return {};

    return QPixmap::fromImage(img);
}

QPixmap toQPixmap(const QByteArray& bytes, Qt::ImageConversionFlags flags)
{
    QPixmap pixmap;
    pixmap.loadFromData(bytes, nullptr, flags);
    return pixmap;
}

QByteArray toQByteArray(const QPixmap& pixmap, const char* format)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, format);
    return bytes;
}

} // namespace Mayo::QtGuiUtils
