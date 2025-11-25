/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qstring_utils.h"

#include <gp_Trsf.hxx>
#include <Precision.hxx>
#include <Quantity_Color.hxx>

namespace Mayo {

static QString valueText(double value, const QStringUtils::TextOptions& opt)
{
    auto fnLastChar = [](const QString& str) {
        return !str.isEmpty() ? str.at(str.size() - 1) : QChar();
    };

    const double c = std::abs(value) < Precision::Confusion() ? 0. : value;
    QString str = opt.locale.toString(c, 'f', opt.unitDecimals);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // TODO Consider QString decimal point
    const QChar chDecPnt = opt.locale.decimalPoint().front();
#else
    const QChar chDecPnt = opt.locale.decimalPoint();
#endif
    const int posPnt = str.indexOf(chDecPnt);
    if (posPnt != -1) { // Remove useless trailing zeroes
        while (fnLastChar(str) == opt.locale.zeroDigit())
            str.chop(1);

        if (fnLastChar(str) == chDecPnt)
            str.chop(1);
    }

    return str;
}

static QString coordsText(const gp_XYZ& coords, const QStringUtils::TextOptions& opt)
{
    const QString strX = valueText(coords.X(), opt);
    const QString strY = valueText(coords.Y(), opt);
    const QString strZ = valueText(coords.Z(), opt);
    return QStringUtils::tr("(%1 %2 %3)").arg(strX, strY, strZ);
}

static QString pntCoordText(double coord, const QStringUtils::TextOptions& opt)
{
    const auto trCoord = UnitSystem::translate(opt.unitSchema, coord * Quantity_Millimeter);
    const QString strValue = valueText(trCoord.value, opt);
    return strValue + trCoord.strUnit;
}

QString QStringUtils::text(double value, const TextOptions& opt)
{
    return valueText(value, opt);
}

QString QStringUtils::text(const gp_Pnt& pos, const TextOptions& opt)
{
    const QString strX = pntCoordText(pos.X(), opt);
    const QString strY = pntCoordText(pos.Y(), opt);
    const QString strZ = pntCoordText(pos.Z(), opt);
    return tr("(%1 %2 %3)").arg(strX, strY, strZ);
}

QString QStringUtils::text(const gp_Dir& dir, const TextOptions& opt)
{
    return coordsText(dir.XYZ(), opt);
}

QString QStringUtils::text(const gp_Trsf& trsf, const TextOptions& opt)
{
    gp_XYZ axisRotation;
    double angleRotation;
    trsf.GetRotation(axisRotation, angleRotation);
    const auto trAngleRotation = UnitSystem::degrees(angleRotation * Quantity_Radian);
    return tr("[%1; %2%3; %4]").arg(
                coordsText(axisRotation, opt),
                valueText(trAngleRotation.value, opt),
                QString::fromUtf8(trAngleRotation.strUnit),
                QStringUtils::text(gp_Pnt(trsf.TranslationPart()), opt)
           );
}

QString QStringUtils::text(const Quantity_Color& color, const QString& format)
{
    const int red = color.Red() * 255;
    const int green = color.Green() * 255;
    const int blue = color.Blue() * 255;
    return format.arg(red).arg(green).arg(blue);
}

QString QStringUtils::bytesText(uint64_t sizeBytes, const QLocale& locale)
{
    constexpr int oneMB = 1024 * 1024;
    const quint64 qSizeBytes = sizeBytes; // Avoids ambiguous calls of QLocale::toString()
    if (sizeBytes < 1024 )
        return tr("%1%2").arg(locale.toString(qSizeBytes), tr("B"));
    else if (sizeBytes < oneMB)
        return tr("%1%2").arg(locale.toString(qSizeBytes / 1024), tr("KB"));
    else if (sizeBytes < (10 * oneMB))
        return tr("%1%2").arg(locale.toString(qSizeBytes / double(oneMB), 'f', 2), tr("MB"));
    else if (sizeBytes < (100 * oneMB))
        return tr("%1%2").arg(locale.toString(qSizeBytes / double(oneMB), 'f', 1), tr("MB"));
    else
        return tr("%1%2").arg(locale.toString(qSizeBytes / oneMB), tr("MB"));
}

QString QStringUtils::yesNoText(bool on)
{
    return on ? tr("Yes") : tr("No");
}

QString QStringUtils::yesNoText(CheckState state)
{
    switch (state) {
    case CheckState::Off: return tr("No");
    case CheckState::Partially: return tr("Partially");
    case CheckState::On: return tr("Yes");
    }
    return {};
}

void QStringUtils::append(QString* dst, const QString& str, const QLocale& locale)
{
    if (locale.textDirection() == Qt::LeftToRight)
        dst->append(str);
    else
        dst->prepend(str);
}

} // namespace Mayo
