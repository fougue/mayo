/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include <TopoDS_Shape.hxx>

namespace Mayo::IO {

// Reader for OpenCascade BRep file format
class OccBRepReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    TopoDS_Shape m_shape;
    FilePath m_baseFilename;
};

// Writer for OpenCascade BRep file format
class OccBRepWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    TopoDS_Shape m_shape;
};

} // namespace Mayo::IO
