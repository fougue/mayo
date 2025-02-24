/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_gmio.h"

#include "io_gmio_amf_writer.h"

#include <gmio_core/version.h>

namespace Mayo::IO {

Span<const Format> GmioFactoryWriter::formats() const
{
    static const Format array[] = { Format_AMF };
    return array;
}

std::unique_ptr<Writer> GmioFactoryWriter::create(Format format) const
{
    if (format == Format_AMF)
        return std::make_unique<GmioAmfWriter>();

    return {};
}

std::unique_ptr<PropertyGroup>
GmioFactoryWriter::createProperties(Format format, PropertyGroup* parentGroup) const
{
    if (format == Format_AMF)
        return GmioAmfWriter::createProperties(parentGroup);

    return {};
}

std::string_view GmioLib::strVersion()
{
    return GMIO_VERSION_STR;
}

} // namespace Mayo::IO
