/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io.h"

namespace Mayo {
namespace IO {

class OccFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Reader> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const override;
};

class OccFactoryWriter : public FactoryWriter {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Writer> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const override;
};

} // namespace IO
} // namespace Mayo
