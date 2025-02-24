/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"
#include "../base/io_single_format_factory.h"
#include "../base/point_cloud_data.h"

#include <Quantity_ColorRGBA.hxx>
#include <vector>

namespace Mayo { class IMeshAccess; }

namespace Mayo::IO {

// Writer for PLY file format
class PlyWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters
    enum class Format { Ascii, Binary };

    struct Parameters {
        Format format = Format::Binary;
        bool writeColors = true;
        Quantity_ColorRGBA defaultColor{ Quantity_Color(Quantity_NOC_GRAY) };
        std::string comment;
        // TODO bool writeNormals = false;
        // TODO bool writeEdges = true;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    struct Vertex { float x; float y; float z; };
    struct Color { uint8_t red; uint8_t green; uint8_t blue; };
    struct Face { int32_t v1; int32_t v2; int32_t v3; };

    static Vertex toVertex(const gp_Pnt& pnt);
    static Color toColor(const Quantity_Color& c);

    void addMesh(const IMeshAccess& mesh);
    void addPointCloud(const PointCloudDataPtr& pntCloud);

    class Properties;
    Parameters m_params;
    std::vector<Vertex> m_vecNode;
    std::vector<Color> m_vecNodeColor;
    std::vector<Face> m_vecFace;
};

// Provides factory to create PlyWriter objects
class PlyFactoryWriter : public SingleFormatFactoryWriter<Format_PLY, PlyWriter> {};

} // namespace Mayo::IO
