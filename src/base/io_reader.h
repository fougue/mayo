/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include "filepath.h"
#include "io_format.h"
#include "span.h"
#include <TDF_LabelSequence.hxx>
#include <memory>

namespace Mayo {

class Messenger;
class PropertyGroup;
class TaskProgress;

namespace IO {

class Reader {
public:
    Reader();
    virtual ~Reader() = default;

    virtual bool readFile(const FilePath& fp, TaskProgress* progress) = 0;
    virtual TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) = 0;
    virtual void applyProperties(const PropertyGroup* /*params*/) {}

    Messenger* messenger() const { return m_messenger; }
    void setMessenger(Messenger* messenger);

private:
    Messenger* m_messenger = nullptr;
};

class FactoryReader {
public:
    virtual ~FactoryReader() = default;
    virtual Span<const Format> formats() const = 0;
    virtual std::unique_ptr<Reader> create(Format format) const = 0;
    virtual std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const = 0;
};

} // namespace IO
} // namespace Mayo
