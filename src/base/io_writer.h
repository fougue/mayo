/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "filepath.h"
#include "io_format.h"
#include "span.h"
#include <memory>

namespace Mayo {

class ApplicationItem;
class PropertyGroup;
class TaskProgressPortion;

namespace IO {

class Writer {
public:
    virtual ~Writer() = default;
    virtual bool transfer(Span<const ApplicationItem> appItems, TaskProgressPortion* progress) = 0;
    virtual bool writeFile(const FilePath& fp, TaskProgressPortion* progress) = 0;
    virtual void applyProperties(const PropertyGroup* /*params*/) {}
};

class FactoryWriter {
public:
    virtual ~FactoryWriter() = default;
    virtual Span<const Format> formats() const = 0;
    virtual std::unique_ptr<Writer> create(const Format& format) const = 0;
    virtual std::unique_ptr<PropertyGroup> createProperties(
            const Format& format,
            PropertyGroup* parentGroup) const = 0;
};

} // namespace IO
} // namespace Mayo
