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
    OccBRepReader(bool binaryMode = false);
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    TopoDS_Shape m_shape;
    FilePath m_baseFilename;
    bool m_isBinary = false;
};

// Writer for OpenCascade BRep file format
class OccBRepWriter : public Writer {
public:
    OccBRepWriter(bool binaryMode = false);
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    TopoDS_Shape m_shape;
    bool m_isBinary = false;
};

} // namespace Mayo::IO
