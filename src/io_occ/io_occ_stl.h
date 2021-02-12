/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include <Poly_Triangulation.hxx>
#include <TopoDS_Shape.hxx>
#include <QtCore/QString>

namespace Mayo {
namespace IO {

// Opencascade-based reader for STL file format
class OccStlReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

private:
    Handle_Poly_Triangulation m_mesh;
    QString m_baseFilename;
};

// Opencascade-based writer for STL file format
class OccStlWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

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
    Handle_Poly_Triangulation m_mesh;
};

} // namespace IO
} // namespace Mayo
