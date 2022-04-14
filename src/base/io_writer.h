/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "filepath.h"
#include "io_format.h"
#include "messenger_client.h"
#include "span.h"
#include <memory>

namespace Mayo {

class ApplicationItem;
class PropertyGroup;
class TaskProgress;

namespace IO {

// Abstract base class for writers
// Provides services for writing files in two steps:
//     - transfer a list of items to be written
//     - write transferred items into target file
class Writer : public MessengerClient {
public:
    virtual ~Writer() = default;

    // Converts items(documents and document nodes) into data ready to be written
    // Returns 'true' on success
    virtual bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) = 0;

    // Writes contents(items passed to transfer()) to the file at path 'fp'
    // Returns 'true' on success
    virtual bool writeFile(const FilePath& fp, TaskProgress* progress) = 0;

    // Apply properties contain in 'group' to the writer's parameter values(known in writer sub-class)
    virtual void applyProperties(const PropertyGroup* group) = 0;
};

// Abstract base class for all writer factories
class FactoryWriter {
public:
    virtual ~FactoryWriter() = default;

    // Returns supported formats, ie the formats this factory can create writers for
    virtual Span<const Format> formats() const = 0;

    // Creates and returns a Writer object that matches the given format, or nullptr if no matching writer is found
    virtual std::unique_ptr<Writer> create(Format format) const = 0;

    // Creates and returns properties that match the given format. Those properties is a generic
    // way to change parameter values of a Writer object corresponding to format(see also Writer::applyProperties())
    virtual std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const = 0;
};

} // namespace IO
} // namespace Mayo
