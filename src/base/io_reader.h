/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include "filepath.h"
#include "io_format.h"
#include "messenger_client.h"

#include <gsl/span>
#include <memory>

namespace Mayo {

class PropertyGroup;
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

    // Returns the mutable parameters used to configure this reader, empty by default
    virtual PropertyGroup& parameters();

    // Returns the read-only parameters of this reader, empty by default
    virtual const PropertyGroup& constParameters() const;
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
    // way to change parameter values of a Reader object corresponding to format(see also Reader::applyParameters())
    virtual std::unique_ptr<PropertyGroup> createParameters(Format format) const = 0;
};

} // namespace IO
} // namespace Mayo
