/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

namespace Mayo { class PropertyGroup; }

namespace Mayo {
namespace IO {

struct Format;

// Abstract mechanism to provide reader/writer parameters for a format
class ParametersProvider {
public:
    virtual const PropertyGroup* findReaderParameters(const Format& format) const = 0;
    virtual const PropertyGroup* findWriterParameters(const Format& format) const = 0;
};

} // namespace IO
} // namespace Mayo
