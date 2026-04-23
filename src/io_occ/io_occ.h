/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/property.h"

namespace Mayo::IO {

// Provides factory for OpenCascade-based Reader objects
class OccFactoryReader : public FactoryReader {
public:
    gsl::span<const Format> formats() const override;
    std::unique_ptr<Reader> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;
};

// Provides factory for OpenCascade-based Writer objects
class OccFactoryWriter : public FactoryWriter {
public:
    gsl::span<const Format> formats() const override;
    std::unique_ptr<Writer> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;
};

} // namespace Mayo::IO
