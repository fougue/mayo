/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
    Span<const Format> formats() const override;
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
