/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include "../base/property.h"
#include <common/mayo_config.h>

#include <memory>

namespace Mayo::IO {

// Provides factory for gmio-based Writer objects
class GmioFactoryWriter : public FactoryWriter {
public:
    gsl::span<const Format> formats() const override;
    std::unique_ptr<Writer> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;

    static std::unique_ptr<FactoryWriter> create() {
#ifdef MAYO_HAVE_GMIO
        return std::make_unique<GmioFactoryWriter>();
#else
        return {};
#endif
    }
};

struct GmioLib {
    static std::string_view strName() { return "gmio"; }
#ifdef MAYO_HAVE_GMIO
    static std::string_view strVersion();
    static std::string_view strVersionDetails() { return "(build)"; }
#else
    static std::string_view strVersion() { return ""; }
    static std::string_view strVersionDetails() { return ""; }
#endif
};

} // namespace Mayo::IO
