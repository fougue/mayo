/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include "filepath.h"
#include "io_format.h"
#include "messenger_client.h"
#include "property.h"

#include <gsl/span>
#include <memory>

namespace Mayo {

class TaskProgress;

namespace IO {

// Abstract base class for readers
// Provides services for reading files in two steps:
//     - parse input file into memory(service Reader::readFile())
//     - convert data in memory into Document object(service Reader::transfer())
class Reader : public MessengerClient {
public:
    virtual ~Reader() = default;

    // Reads file at path 'fp' into memory using indicator to report progress
    // Returns 'true' on success
    virtual bool readFile(const FilePath& fp, TaskProgress* progress) = 0;

    // Converts data read during readFile() step into document 'doc' using indicator to report progress
    // Returns the list of entities added to document 'doc'
    virtual NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) = 0;
};

// Provides a rich, editable representation of a Reader's parameters
// This is the bridge between the Property system and the concrete parameters of a Reader (if any)
// It allows properties to be loaded from a Reader and later saved back to it, while keeping the
// Reader itself independent from the Property system
class ReaderProperties : public PropertyGroup {
public:
    // Saves the current property values to the corresponding Reader
    // Returns true if all properties were successfully applied, false otherwise
    // The Reader must be of the type supported by the concrete WriterProperties implementation
    virtual bool saveTo(Reader& reader) const = 0;

    // Loads the property values from the corresponding Reader
    // Returns true if all property values were successfully loaded, false otherwise
    // The Reader must be of the type supported by the concrete WriterProperties implementation
    virtual bool loadFrom(const Reader& reader) = 0;
};

// Abstract base class for all reader factories
class FactoryReader {
public:
    virtual ~FactoryReader() = default;

    // Returns supported formats, ie the formats this factory can create readers for
    virtual gsl::span<const Format> formats() const = 0;

    // Creates and returns a Reader object that matches the given format, or nullptr if no matching reader is found
    virtual std::unique_ptr<Reader> create(Format format) const = 0;

    // Creates and returns parameters that match the given format. Those parameters is a generic
    // way to change parameter values of a Reader object corresponding to format
    virtual std::unique_ptr<ReaderProperties> createProperties(Format format) const = 0;
};

} // namespace IO
} // namespace Mayo
