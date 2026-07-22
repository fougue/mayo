/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/occ_handle.h"
#include <Poly_Triangulation.hxx>
#include <TopoDS_Shape.hxx>

namespace Mayo::IO {

// Opencascade-based reader for STL file format
class OccStlReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    OccHandle<Poly_Triangulation> m_mesh;
    FilePath m_baseFilename;
};

// Opencascade-based writer for STL file format
class OccStlWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters
    enum class Format { Ascii, Binary };

    struct Parameters {
        Format format = Format::Binary;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    class Properties;
    Parameters m_params;
    TopoDS_Shape m_shape;
};

} // namespace Mayo::IO
