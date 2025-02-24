/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/io_reader.h"
#include "../base/property.h"
#include <common/mayo_config.h>

#include <memory>

namespace Mayo::IO {

// Provides factory for Assimp-based Reader objects
class AssimpFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Reader> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;

    static std::unique_ptr<FactoryReader> create() {
#ifdef MAYO_HAVE_ASSIMP
        return std::make_unique<AssimpFactoryReader>();
#else
        return {};
#endif
    }
};

struct AssimpLib {
    static std::string_view strName() { return "Assimp"; }
#ifdef MAYO_HAVE_ASSIMP
    static std::string_view strVersion();
    static std::string_view strVersionDetails();
#else
    static std::string_view strVersion() { return ""; }
    static std::string_view strVersionDetails() { return ""; }
#endif
};

} // namespace Mayo::IO
