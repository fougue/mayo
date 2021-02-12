/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Quantity_Color.hxx>
#include <QtCore/QMetaType>
Q_DECLARE_METATYPE(Quantity_Color)

class QDataStream;
QDataStream& operator<<(QDataStream& out, const Quantity_Color& color);
QDataStream& operator>>(QDataStream& in, Quantity_Color& color);

namespace Mayo {

void qtRegisterMetaType_OccColor();

} // namespace Mayo
