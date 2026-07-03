/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"

namespace Mayo::IO {

template<Format Fmt, typename FormatReader>
class SingleFormatFactoryReader : public FactoryReader {
public:
    gsl::span<const Format> formats() const override;
    std::unique_ptr<Reader> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;
};

template<Format Fmt, typename FormatWriter>
class SingleFormatFactoryWriter : public FactoryWriter {
public:
    gsl::span<const Format> formats() const override;
    std::unique_ptr<Writer> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;
};

// --
// -- Implementation
// --

template<Format Fmt, typename FormatReader>
gsl::span<const Format> SingleFormatFactoryReader<Fmt, FormatReader>::formats() const
{
    static const Format arrayFormat[] = { Fmt };
    return arrayFormat;
}

template<Format Fmt, typename FormatReader>
std::unique_ptr<Reader> SingleFormatFactoryReader<Fmt, FormatReader>::create(Format format) const
{
    if (format == Fmt)
        return std::make_unique<FormatReader>();
    else
        return {};
}

template<Format Fmt, typename FormatReader>
std::unique_ptr<PropertyGroup>
SingleFormatFactoryReader<Fmt, FormatReader>::createProperties(Format format, PropertyGroup* parentGroup) const
{
    if (format == Fmt)
        return FormatReader::createProperties(parentGroup);
    else
        return {};
}

template<Format Fmt, typename FormatWriter>
gsl::span<const Format> SingleFormatFactoryWriter<Fmt, FormatWriter>::formats() const
{
    static const Format arrayFormat[] = { Fmt };
    return arrayFormat;
}

template<Format Fmt, typename FormatWriter>
std::unique_ptr<Writer> SingleFormatFactoryWriter<Fmt, FormatWriter>::create(Format format) const
{
    if (format == Fmt)
        return std::make_unique<FormatWriter>();
    else
        return {};
}

template<Format Fmt, typename FormatWriter>
std::unique_ptr<PropertyGroup>
SingleFormatFactoryWriter<Fmt, FormatWriter>::createProperties(Format format, PropertyGroup* parentGroup) const
{
    if (format == Fmt)
        return FormatWriter::createProperties(parentGroup);
    else
        return {};
}

} // namespace Mayo::IO
