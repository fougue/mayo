/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"

#include <RWGltf_WriterTrsfFormat.hxx>
#include <RWMesh_CoordinateSystemConverter.hxx>

namespace Mayo::IO {

// OpenCascade-based writer for glTF format
// Requires OpenCascade >= v7.5.0
class OccGltfWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> spanAppItem, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters
    enum class Format { Json, Binary };
    enum class ShapeNameFormat {
        Empty,    // Omit name
        Product,  // Product name(eg from XCaf::shapeReferred(), which could be shared by multiple instances)
        Instance,
        InstanceOrProduct,  // Instance name when available and Product name otherwise
        ProductOrInstance,  // Product name when available and Instance name otherwise
        ProductAndInstance, // Generates "Product [Instance]" name
    };

    struct Parameters {
        RWMesh_CoordinateSystem inputCoordinateSystem  = RWMesh_CoordinateSystem_Undefined;
        RWMesh_CoordinateSystem outputCoordinateSystem = RWMesh_CoordinateSystem_glTF;
        RWGltf_WriterTrsfFormat transformationFormat = RWGltf_WriterTrsfFormat_Compact;
        Format format = Format::Binary;
        bool forceExportUV = true;
#if OCC_VERSION_HEX >= 0x070600
        OccGltfWriter::ShapeNameFormat nodeNameFormat = ShapeNameFormat::ProductOrInstance;
        OccGltfWriter::ShapeNameFormat meshNameFormat = ShapeNameFormat::Product;
#else
        OccGltfWriter::ShapeNameFormat nodeNameFormat = ShapeNameFormat::Empty;
        OccGltfWriter::ShapeNameFormat meshNameFormat = ShapeNameFormat::Empty;
#endif
        bool mergeFaces = false;
        bool embedTextures = true;   // ⚠ Only applicable if `format` == Format::Binary
        bool keepIndices16b = false; // ⚠ Only applicable if `mergeFaces` == true
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    Parameters m_params;
    DocumentPtr m_document;
    NCollection_Sequence<TDF_Label> m_seqRootLabel;
};

} // namespace Mayo::IO
