#include "string_utils.h"

#include "fougtools/occtools/qt_utils.h"
#include <gp_Trsf.hxx>
#include <Quantity_Color.hxx>
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
    case TopAbs_COMPOUND: return "COMPOUND";
    case TopAbs_COMPSOLID: return "COMPSOLID";
    case TopAbs_SOLID: return "SOLID";
    case TopAbs_SHELL: return "SHELL";
    case TopAbs_FACE: return "FACE";
    case TopAbs_WIRE: return "WIRE";
    case TopAbs_EDGE: return "EDGE";
    case TopAbs_VERTEX: return "VERTEX";
    case TopAbs_SHAPE: return "SHAPE";
    }
    return "??";
}

} // namespace Mayo
