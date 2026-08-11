/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"
#include "../base/io_single_format_factory.h"
#include "../base/point_cloud_data.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"

#include <Quantity_ColorRGBA.hxx>
#include <vector>

namespace Mayo { class IMeshAccess; }

namespace Mayo::IO {

// Writer for PLY file format
class PlyWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters
    enum class Format { Ascii, Binary };

    struct Parameters : public PropertyGroup {
        PropertyEnum<PlyWriter::Format> format{ this, textId("format") };
        PropertyBool writeColors{ this, textId("writeColors") };
        PropertyOccColor defaultColor{ this, textId("defaultColor") };
        PropertyString comment{ this, textId("comment") };
        // TODO bool writeNormals = false;
        // TODO bool writeEdges = true;

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::PlyWriter)

    struct Vertex { float x; float y; float z; };
    struct Color { uint8_t red; uint8_t green; uint8_t blue; };
    struct Face { int32_t v1; int32_t v2; int32_t v3; };

    static Vertex toVertex(const gp_Pnt& pnt);
    static Color toColor(const Quantity_Color& c);

    void addMesh(const IMeshAccess& mesh);
    void addPointCloud(const PointCloudDataPtr& pntCloud);

    Parameters m_params;
    std::vector<Vertex> m_vecNode;
    std::vector<Color> m_vecNodeColor;
    std::vector<Face> m_vecFace;
};

// Provides factory to create PlyWriter objects
class PlyFactoryWriter : public SingleFormatFactoryWriter<Format_PLY, PlyWriter> {};

} // namespace Mayo::IO
