/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include "filepath.h"
#include "io_format.h"
#include "messenger_client.h"
#include "span.h"
#include <TDF_LabelSequence.hxx>
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
    virtual TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) = 0;

    // Apply properties contain in 'group' to the reader's parameter values(known in reader sub-class)
    virtual void applyProperties(const PropertyGroup* group) = 0;
};

// Abstract base class for all reader factories
class FactoryReader {
public:
    virtual ~FactoryReader() = default;

    // Returns supported formats, ie the formats this factory can create readers for
    virtual Span<const Format> formats() const = 0;

    // Creates and returns a Reader object that matches the given format, or nullptr if no matching reader is found
    virtual std::unique_ptr<Reader> create(Format format) const = 0;

    // Creates and returns properties that match the given format. Those properties is a generic
    // way to change parameter values of a Reader object corresponding to format(see also Reader::applyProperties())
    virtual std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const = 0;
};

} // namespace IO
} // namespace Mayo
