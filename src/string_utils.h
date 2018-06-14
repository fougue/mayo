/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "unit_system.h"
#include <QtCore/QString>
#include <IFSelect_ReturnStatus.hxx>
#include <TopAbs_ShapeEnum.hxx>
class Quantity_Color;
class gp_Pnt;
class gp_Trsf;

namespace Mayo {

struct StringUtils {
    static QString text(
            const gp_Pnt& pos,
            UnitSystem::Schema schema = UnitSystem::SI);
    static QString text(
            const gp_Trsf& trsf,
            UnitSystem::Schema schema = UnitSystem::SI);
    static QString text(
            const Quantity_Color& color,
            const QString& format = "R:%1 G:%2 B:%3");
    static const char* rawText(TopAbs_ShapeEnum shapeType);
    static const char* rawText(IFSelect_ReturnStatus status);

    static const char* skipWhiteSpaces(const char* str, size_t len);
};

} // namespace Mayo
