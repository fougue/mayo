/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qmeta_quantity_color.h"
#include <QtCore/QDataStream>

QDataStream& operator<<(QDataStream& out, const Quantity_Color& color)
{
    out << color.Red() << color.Green() << color.Blue();
    return out;
}

QDataStream& operator>>(QDataStream& in, Quantity_Color& color)
{
    double r, g, b;
    in >> r;
    in >> g;
    in >> b;
    color.SetValues(r, g, b, Quantity_TOC_RGB);
    return in;
}

void Mayo::qtRegisterMetaType_OccColor()
{
    qRegisterMetaType<Quantity_Color>("Quantity_Color");
    qRegisterMetaTypeStreamOperators<Quantity_Color>("Quantity_Color");
}
