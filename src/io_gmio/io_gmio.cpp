/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_gmio.h"

#include "io_gmio_amf_writer.h"

namespace Mayo {
namespace IO {

Span<const Format> GmioFactoryWriter::formats() const
{
    static const Format array[] = { Format_AMF };
    return array;
}

std::unique_ptr<Writer> GmioFactoryWriter::create(const Format& format) const
{
    if (format == Format_AMF)
        return std::make_unique<GmioAmfWriter>();

    return {};
}

std::unique_ptr<PropertyGroup>
GmioFactoryWriter::createProperties(const Format& format, PropertyGroup* parentGroup) const
{
    if (format == Format_AMF)
        return GmioAmfWriter::createProperties(parentGroup);

    return {};
}

} // namespace IO
} // namespace Mayo
