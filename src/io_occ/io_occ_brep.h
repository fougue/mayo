/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include <TopoDS_Shape.hxx>

namespace Mayo::IO {

// Reader for OpenCascade BRep file format
class OccBRepReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccBRepReader)
    TopoDS_Shape m_shape;
    FilePath m_baseFilename;
};

// Writer for OpenCascade BRep file format
class OccBRepWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters
    enum class Format { Ascii, Binary };

    struct Parameters : public PropertyGroup {
        PropertyEnum<Format> format{ this, textId("format") };
        PropertyBool saveShapeTriangulation{ this, textId("saveShapeTriangulation") };
        PropertyBool saveShapeTriangulationNormals{ this, textId("saveShapeTriangulationNormals") };

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccBRepWriter)
    Parameters m_params;
    TopoDS_Shape m_shape;
};

} // namespace Mayo::IO
