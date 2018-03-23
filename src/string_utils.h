#pragma once

#include <QtCore/QString>
#include <TopAbs_ShapeEnum.hxx>
class Quantity_Color;
class gp_Trsf;

namespace Mayo {

class StringUtils {
public:
    static QString text(const gp_Trsf& trsf);
    static QString text(
            const Quantity_Color& color, const QString& format = "R:%1 G:%2 B:%3");
    static const char* rawText(TopAbs_ShapeEnum shapeType);
};

} // namespace Mayo
