/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include "document_ptr.h"
#include <memory>
class QString;

namespace Mayo {
class PropertyGroup;
class TaskProgress;
}

namespace Mayo {
namespace IO {

struct Format;

class Reader {
public:
    // TODO Replace QString with std::filesystem::path
    virtual bool readFile(const QString& filepath, TaskProgress* progress) = 0;
    virtual bool transfer(DocumentPtr doc, TaskProgress* progress) = 0;
    virtual void applyProperties(const PropertyGroup* /*params*/) {}
};

class FactoryReader {
public:
    virtual Span<const Format> formats() const = 0;
    virtual std::unique_ptr<Reader> create(const Format& format) const = 0;
    virtual std::unique_ptr<PropertyGroup> createProperties(
            const Format& format,
            PropertyGroup* parentGroup) const = 0;
};

} // namespace IO
} // namespace Mayo
