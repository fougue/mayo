/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_format.h"

#include <QtCore/QHash>

namespace Mayo {
namespace IO {

bool operator==(const Format& lhs, const Format& rhs) {
    return lhs.identifier.compare(rhs.identifier, Qt::CaseInsensitive) == 0;
}

bool operator!=(const Format& lhs, const Format& rhs) {
    return lhs.identifier.compare(rhs.identifier, Qt::CaseInsensitive) != 0;
}

unsigned hash(const Format& key)
{
    return qHash(key.identifier.toLower());
}

} // namespace IO
} // namespace Mayo
