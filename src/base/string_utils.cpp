/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "string_utils.h"

#include "unit_system.h"
#include <gp_Trsf.hxx>
#include <Precision.hxx>
#include <Quantity_Color.hxx>
#include <cctype>

namespace Mayo {

static QChar lastChar(const QString& str)
{
    return str.at(str.size() - 1);
}

static QString valueText(double value, const StringUtils::TextOptions& opt)
{
    const double c = std::abs(value) < Precision::Confusion() ? 0. : value;
    QString str = opt.locale.toString(c, 'f', opt.unitDecimals);
    const QChar chDecPnt = opt.locale.decimalPoint();
    const int posPnt = str.indexOf(chDecPnt);
    if (posPnt != -1) { // Remove useless trailing zeroes
        while (lastChar(str) == opt.locale.zeroDigit())
            str.chop(1);
        if (lastChar(str) == chDecPnt)
            str.chop(1);
    }
    return str;
}

static QString coordsText(const gp_XYZ& coords, const StringUtils::TextOptions& opt)
{
    const QString strX = valueText(coords.X(), opt);
    const QString strY = valueText(coords.Y(), opt);
    const QString strZ = valueText(coords.Z(), opt);
    return StringUtils::tr("(%1 %2 %3)").arg(strX, strY, strZ);
}

static QString pntCoordText(double coord, const StringUtils::TextOptions& opt)
{
    const UnitSystem::TranslateResult trCoord =
            UnitSystem::translate(opt.unitSchema, coord * Quantity_Millimeter);
    const QString strValue = valueText(trCoord.value, opt);
    return strValue + trCoord.strUnit;
}

QString StringUtils::text(double value, const TextOptions& opt)
{
    return valueText(value, opt);
}

QString StringUtils::text(const gp_Pnt& pos, const TextOptions& opt)
{
    const QString strX = pntCoordText(pos.X(), opt);
    const QString strY = pntCoordText(pos.Y(), opt);
    const QString strZ = pntCoordText(pos.Z(), opt);
    return tr("(%1 %2 %3)").arg(strX, strY, strZ);
}

QString StringUtils::text(const gp_Dir& dir, const TextOptions& opt)
{
    return coordsText(dir.XYZ(), opt);
}

QString StringUtils::text(const gp_Trsf& trsf, const TextOptions& opt)
{
    gp_XYZ axisRotation;
    double angleRotation;
    trsf.GetRotation(axisRotation, angleRotation);
    const UnitSystem::TranslateResult trAngleRotation =
            UnitSystem::degrees(angleRotation * Quantity_Radian);
    return tr("[%1; %2%3; %4]").arg(
                coordsText(axisRotation, opt),
                valueText(trAngleRotation.value, opt),
                QString::fromUtf8(trAngleRotation.strUnit),
                StringUtils::text(gp_Pnt(trsf.TranslationPart()), opt));
}

QString StringUtils::text(const Quantity_Color& color, const QString& format)
{
    const int red = color.Red() * 255;
    const int green = color.Green() * 255;
    const int blue = color.Blue() * 255;
    return format.arg(red).arg(green).arg(blue);
}

const char* StringUtils::rawText(TopAbs_ShapeEnum shapeType)
{
    // TODO Replace maybe by TopAbs::ShapeTypeToString()
    switch (shapeType) {
    case TopAbs_COMPOUND: return "TopAbs_COMPOUND";
    case TopAbs_COMPSOLID: return "TopAbs_COMPSOLID";
    case TopAbs_SOLID: return "TopAbs_SOLID";
    case TopAbs_SHELL: return "TopAbs_SHELL";
    case TopAbs_FACE: return "TopAbs_FACE";
    case TopAbs_WIRE: return "TopAbs_WIRE";
    case TopAbs_EDGE: return "TopAbs_EDGE";
    case TopAbs_VERTEX: return "TopAbs_VERTEX";
    case TopAbs_SHAPE: return "TopAbs_SHAPE";
    }
    return "??";
}

const char* StringUtils::rawText(IFSelect_ReturnStatus status)
{
    switch (status) {
    case IFSelect_RetVoid: return "IFSelect_RetVoid";
    case IFSelect_RetDone: return "IFSelect_RetDone";
    case IFSelect_RetError: return "IFSelect_RetError";
    case IFSelect_RetFail: return "IFSelect_RetFail";
    case IFSelect_RetStop: return "IFSelect_RetStop";
    }
    return "??";
}

void StringUtils::append(QString* dst, const QString& str, const QLocale& locale)
{
    if (locale.textDirection() == Qt::LeftToRight)
        dst->append(str);
    else
        dst->prepend(str);
}

} // namespace Mayo
