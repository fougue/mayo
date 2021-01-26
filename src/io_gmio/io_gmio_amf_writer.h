/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"

#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <TopLoc_Location.hxx>

#include <gmio_amf/amf_document.h>
#include <string>
#include <vector>

namespace Mayo {
namespace IO {

// gmio-based writer for AMF format
// Requires gmio >= v0.4.0
class GmioAmfWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> spanAppItem, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* group) override;

    // Parameters

    enum class FloatTextFormat {
        Decimal, // -> GMIO_FLOAT_TEXT_FORMAT_DECIMAL_UPPERCASE
        Scientific, // -> GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_UPPERCASE
        Shortest // -> GMIO_FLOAT_TEXT_FORMAT_SHORTEST_UPPERCASE
    };

    struct Parameters {
        // TODO gmio_amf_unit
        FloatTextFormat float64Format = FloatTextFormat::Decimal;
        uint8_t float64Precision = 16;
        bool createZipArchive = false;
        bool useZip64 = true;
        std::string zipEntryFilename; // UTF8
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    int createObject(const TDF_Label& labelShape);

    static const GmioAmfWriter* from(const void* cookie);

    static void amf_getDocumentElement(
            const void* cookie,
            enum gmio_amf_document_element element,
            uint32_t elementIndex,
            void* ptrElement);

    static void amf_getDocumentElementMetadata(
            const void* cookie,
            enum gmio_amf_document_element element,
            uint32_t elementIndex,
            uint32_t metadataIndex,
            struct gmio_amf_metadata* ptrMetadata);

    static void amf_getObjectMesh(
            const void* cookie,
            uint32_t objectIndex,
            uint32_t meshIndex,
            struct gmio_amf_mesh* ptrMesh);

    static void amf_getObjectMeshElement(
            const void* cookie,
            const struct gmio_amf_object_mesh_element_index* elementIndex,
            void* ptrElement);

    static void amf_getObjectMeshVolumeTriangle(
            const void* cookie,
            const struct gmio_amf_object_mesh_element_index* volumeIndex,
            uint32_t triangleIndex,
            struct gmio_amf_triangle* ptrTriangle);

    static void amf_getConstellationInstance(
            const void* cookie,
            uint32_t constellationIndex,
            uint32_t instanceIndex,
            struct gmio_amf_instance* ptrInstance);

    struct Instance {
        int objectId = -1;
        gp_Trsf trsf;
        std::string name;
    };

    struct Material {
        int id = -1;
        Quantity_Color color;
        bool isColor = false;
    };

    struct Object {
        int id = -1;
        int firstMeshId = 0;
        int lastMeshId = -1;
        int materialId = -1;
        std::string name;
    };

    struct Mesh {
        int id = -1;
        Handle_Poly_Triangulation triangulation;
        TopLoc_Location location;
        int materialId = -1;
    };

    class Properties;
    Parameters m_params;
    std::vector<Material> m_vecMaterial;
    std::vector<Mesh> m_vecMesh;
    std::vector<Object> m_vecObject;
    std::vector<Instance> m_vecInstance;
};

} // namespace IO
} // namespace Mayo

