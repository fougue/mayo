/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include "../base/property.h"
#include <memory>

namespace Mayo {
namespace IO {

// Provides factory for gmio-based Writer objects
class GmioFactoryWriter : public FactoryWriter {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Writer> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createProperties(
            const Format& format,
            PropertyGroup* parentGroup) const override;

    static std::unique_ptr<FactoryWriter> create() {
#ifdef HAVE_GMIO
        return std::make_unique<GmioFactoryWriter>();
#else
        return {};
#endif
    }
};

} // namespace IO
} // namespace Mayo
