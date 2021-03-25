/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/property.h"
#include <memory>

namespace Mayo {
namespace IO {

// Provides factory for Assimp-based Reader objects
class AssimpFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Reader> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createProperties(
            const Format& format,
            PropertyGroup* parentGroup) const override;

    static std::unique_ptr<FactoryReader> create() {
#ifdef HAVE_ASSIMP
        return std::make_unique<AssimpFactoryReader>();
#else
        return {};
#endif
    }
};

} // namespace IO
} // namespace Mayo
