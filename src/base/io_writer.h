/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "filepath.h"
#include "io_format.h"
#include "messenger_client.h"
#include <gsl/span>
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
    virtual bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) = 0;

    // Writes contents(items passed to transfer()) to the file at path 'fp'
    // Returns 'true' on success
    virtual bool writeFile(const FilePath& fp, TaskProgress* progress) = 0;

    // Returns the mutable parameters used to configure this writer, empty by default
    virtual PropertyGroup& parameters();

    // Returns the read-only parameters of this writer, empty by default
    virtual const PropertyGroup& constParameters() const;
};

// Abstract base class for all writer factories
class FactoryWriter {
public:
    virtual ~FactoryWriter() = default;

    // Returns supported formats, ie the formats this factory can create writers for
    virtual gsl::span<const Format> formats() const = 0;

    // Creates and returns a Writer object that matches the given format, or nullptr if no matching writer is found
    virtual std::unique_ptr<Writer> create(Format format) const = 0;

    // Creates and returns parameters that match the given format. Those parameters is a generic
    // way to change parameter values of a Writer object corresponding to format(see also Writer::applyParameters())
    virtual std::unique_ptr<PropertyGroup> createParameters(Format format) const = 0;
};

} // namespace IO
} // namespace Mayo
