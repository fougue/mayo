/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_format.h"

namespace Mayo { class PropertyGroup; }

namespace Mayo::IO {

// Abstract mechanism to provide reader/writer parameters for a format
class ParametersProvider {
public:
    virtual const PropertyGroup* findReaderParameters(Format format) const = 0;
    virtual const PropertyGroup* findWriterParameters(Format format) const = 0;
};

} // namespace Mayo::IO
