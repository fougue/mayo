/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/property.h"

namespace Mayo {
namespace IO {

// Provides factory for OpenCascade-based Reader objects
class OccFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Reader> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createProperties(
            const Format& format,
            PropertyGroup* parentGroup) const override;
};

// Provides factory for OpenCascade-based Writer objects
class OccFactoryWriter : public FactoryWriter {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Writer> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createProperties(
            const Format& format,
            PropertyGroup* parentGroup) const override;
};

} // namespace IO
} // namespace Mayo
