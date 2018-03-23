#include "string_utils.h"

#include "fougtools/occtools/qt_utils.h"
#include <gp_Trsf.hxx>
#include <Quantity_Color.hxx>
#include <cctype>
#include <cmath>

namespace Mayo {

static const double pi = std::acos(-1);

QString StringUtils::text(const gp_Trsf& trsf)
{
    gp_XYZ axisRotation;
    double angleRotation;
    const gp_XYZ& pos = trsf.TranslationPart();
    trsf.GetRotation(axisRotation, angleRotation);
    return QString("[%1; %2°; %3]")
            .arg(occ::QtUtils::toQString(axisRotation, "(%x %y %z)"))
            .arg((angleRotation * 180) / pi)
            .arg(occ::QtUtils::toQString(pos, "(%xmm %ymm %zmm)"));
}

QString StringUtils::text(const Quantity_Color &color, const QString &format)
{
    return format.arg(color.Red()).arg(color.Green()).arg(color.Blue());
}

const char* StringUtils::rawText(TopAbs_ShapeEnum shapeType)
{
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

const char *StringUtils::skipWhiteSpaces(const char *str, size_t len)
{
    size_t pos = 0;
    while (std::isspace(str[pos]) && pos < len)
        ++pos;
    return str + pos;
}

} // namespace Mayo
