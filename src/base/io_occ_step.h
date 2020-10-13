/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_reader.h"
#include "io_writer.h"
#include <NCollection_Vector.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>

namespace Mayo {
namespace IO {

// Opencascade-based reader for STEP file format
class OccStepReader : public Reader {
public:
    OccStepReader();
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

private:
    STEPCAFControl_Reader m_reader;
};

// Opencascade-based reader for STEP file format
class OccStepWriter : public Writer {
public:
    OccStepWriter();
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

private:
    STEPCAFControl_Writer m_writer;
};

} // namespace IO
} // namespace Mayo
