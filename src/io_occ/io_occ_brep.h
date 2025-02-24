/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    TopoDS_Shape m_shape;
};

} // namespace Mayo::IO
