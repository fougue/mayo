/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_gmio.h"

#include "io_gmio_amf_writer.h"

#include <gmio_core/version.h>

namespace Mayo::IO {

gsl::span<const Format> GmioFactoryWriter::formats() const
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
