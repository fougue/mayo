/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "text_id.h"

#include <QtCore/QCoreApplication>
#include <cstring>

namespace Mayo {

QString TextId::tr() const
{
    return QCoreApplication::translate(this->trContext.data(), this->key.data());
}

bool TextId::isEmpty() const
{
    return this->key.isEmpty();
}

} // namespace Mayo
