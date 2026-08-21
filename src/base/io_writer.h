/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "filepath.h"
#include "io_format.h"
#include "messenger_client.h"
#include "property.h"

#include <gsl/span>
#include <memory>

namespace Mayo {

class ApplicationItem;
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
};

// Provides a rich, editable representation of a Writer's parameters
// This is the bridge between the Property system and the concrete parameters of a Writer (if any)
// It allows properties to be loaded from a Writer and later saved back to it, while keeping the
// Writer itself independent from the Property system
class WriterProperties : public PropertyGroup {
public:
    // Saves the current property values to the corresponding Writer
    // Returns true if all properties were successfully applied, false otherwise
    // The Writer must be of the type supported by the concrete WriterProperties implementation
    virtual bool saveTo(Writer& writer) const = 0;

    // Loads the property values from the corresponding Writer
    // Returns true if all property values were successfully loaded, false otherwise
    // The Writer must be of the type supported by the concrete WriterProperties implementation
    virtual bool loadFrom(const Writer& writer) = 0;
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
    // way to change parameter values of a Writer object corresponding to format
    virtual std::unique_ptr<WriterProperties> createProperties(Format format) const = 0;
};

} // namespace IO
} // namespace Mayo
