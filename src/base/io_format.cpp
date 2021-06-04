/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_format.h"

#include <QtCore/QHash>
#include <algorithm>

namespace Mayo {
namespace IO {

bool operator==(const Format& lhs, const Format& rhs) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return lhs.identifier.compare(rhs.identifier, Qt::CaseInsensitive) == 0;
#else
    return qstrnicmp(lhs.identifier.constData(), rhs.identifier.constData(), lhs.identifier.size()) == 0;
#endif
}

bool operator!=(const Format& lhs, const Format& rhs) {
    return !(lhs == rhs);
}

unsigned hash(const Format& key)
{
    return qHash(key.identifier.toLower());
}

bool formatProvidesBRep(const Format& format)
{
    static const Format brepFormats[] = { Format_STEP, Format_IGES, Format_OCCBREP };
    return std::any_of(
                std::cbegin(brepFormats),
                std::cend(brepFormats),
                [&](const Format& candidate) { return candidate == format; });
}

bool formatProvidesMesh(const Format& format)
{
    return !formatProvidesBRep(format) && format != Format_Unknown;
}

} // namespace IO
} // namespace Mayo
