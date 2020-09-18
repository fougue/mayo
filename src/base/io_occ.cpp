/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ.h"
#include "io_occ_brep.h"
#include "io_occ_iges.h"
#include "io_occ_step.h"
#include "tkernel_utils.h"

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include "io_occ_obj.h"
#endif

#include <fougtools/occtools/qt_utils.h>
#include <regex>

namespace Mayo {
namespace IO {

Span<const Format> OccFactoryReader::formats() const
{
    static const Format array[] = {
        Format_STEP,
        Format_IGES,
        Format_OCCBREP,
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        Format_OBJ,
#endif
        Format_STL
    };
    return array;
}

Format OccFactoryReader::findFormatFromContents(QByteArray contentsBegin, uint64_t hintFullContentsSize) const
{
    // -- Binary STL ?
    constexpr size_t binaryStlHeaderSize = 80 + sizeof(uint32_t);
    if (contentsBegin.size() >= binaryStlHeaderSize) {
        constexpr uint32_t offset = 80; // Skip header
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(contentsBegin.data());
        const uint32_t facetsCount =
                bytes[offset]
                | (bytes[offset+1] << 8)
                | (bytes[offset+2] << 16)
                | (bytes[offset+3] << 24);
        constexpr unsigned facetSize = (sizeof(float) * 12) + sizeof(uint16_t);
        if ((facetSize * facetsCount + binaryStlHeaderSize) == hintFullContentsSize)
            return Format_STL;
    }

    // -- IGES ?
    {
        // regex : ^.{72}S\s*[0-9]+\s*[\n\r\f]
        bool isIges = true;
        if (contentsBegin.size() >= 80 && contentsBegin[72] == 'S') {
            for (int i = 73; i < 80 && isIges; ++i) {
                if (contentsBegin[i] != ' ' && !std::isdigit(static_cast<unsigned char>(contentsBegin[i])))
                    isIges = false;
            }

            const char c80 = contentsBegin[80];
            if (isIges && (c80 == '\n' || c80 == '\r' || c80 == '\f')) {
                const int sVal = std::atoi(contentsBegin.data() + 73);
                if (sVal == 1)
                    return Format_IGES;
            }
        }
    } // IGES

    const std::locale& cLocale = std::locale::classic();
    auto fnIsSpace = [&](char c) { return std::isspace(c, cLocale); };
    auto fnMatchToken = [](QByteArray::const_iterator itBegin, std::string_view token) {
        return std::strncmp(&(*itBegin), token.data(), token.size()) == 0;
    };
    auto itContentsBegin = std::find_if_not(contentsBegin.cbegin(), contentsBegin.cend(), fnIsSpace);

    // -- STEP ?
    {
        // regex : ^\s*ISO-10303-21\s*;\s*HEADER
        constexpr std::string_view stepIsoId = "ISO-10303-21";
        constexpr std::string_view stepHeaderToken = "HEADER";
        if (fnMatchToken(itContentsBegin, stepIsoId)) {
            auto itChar = std::find_if_not(itContentsBegin + stepIsoId.size(), contentsBegin.cend(), fnIsSpace);
            if (itChar != contentsBegin.cend() && *itChar == ';') {
                itChar = std::find_if_not(itChar + 1, contentsBegin.cend(), fnIsSpace);
                if (fnMatchToken(itChar, stepHeaderToken))
                    return Format_STEP;
            }
        }
    } // STEP

    // -- OpenCascade BREP ?
    {
        // regex : ^\s*DBRep_DrawableShape
        constexpr std::string_view occBRepToken = "DBRep_DrawableShape";
        if (fnMatchToken(itContentsBegin, occBRepToken))
            return Format_OCCBREP;
    }

    // -- ASCII STL ?
    {
        // regex : ^\s*solid
        constexpr std::string_view asciiStlToken = "solid";
        if (fnMatchToken(itContentsBegin, asciiStlToken))
            return Format_STL;
    }

    // -- OBJ ?
    {
        const std::regex rx("^\\s*(v|vt|vn|vp|surf)\\s+[-\\+]?[0-9\\.]+\\s");
        if (std::regex_search(contentsBegin.cbegin(), contentsBegin.cend(), rx))
            return Format_OBJ;
    }

    // Fallback case
    return Format_Unknown;
}

std::unique_ptr<Reader> OccFactoryReader::create(const Format& format) const
{
    if (format == Format_STEP)
        return std::make_unique<OccStepReader>();
    else if (format == Format_IGES)
        return std::make_unique<OccIgesReader>();
    else if (format == Format_OCCBREP)
        return std::make_unique<OccBRepReader>();
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    else if (format == Format_OBJ)
        return std::make_unique<OccObjReader>();
#endif

    return {};
}

std::unique_ptr<PropertyGroup> OccFactoryReader::createParameters(
        const Format& format, PropertyGroup* parentGroup) const
{
    return {};
}

bool OccFactoryReader::applyParameters(const PropertyGroup& params, Reader* reader) const
{
    return false;
}

} // namespace IO
} // namespace Mayo
