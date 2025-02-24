/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"

#include <RWGltf_WriterTrsfFormat.hxx>
#include <RWMesh_CoordinateSystemConverter.hxx>
#include <TDF_LabelSequence.hxx>

namespace Mayo::IO {

// OpenCascade-based writer for glTF format
// Requires OpenCascade >= v7.5.0
class OccGltfWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> spanAppItem, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

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
        RWMesh_CoordinateSystem inputCoordinateSystem = RWMesh_CoordinateSystem_Undefined;
        RWMesh_CoordinateSystem outputCoordinateSystem = RWMesh_CoordinateSystem_glTF;
        RWGltf_WriterTrsfFormat transformationFormat = RWGltf_WriterTrsfFormat_Compact;
        Format format = Format::Binary;
        bool forceExportUV = false;
        ShapeNameFormat nodeNameFormat = ShapeNameFormat::ProductOrInstance;
        ShapeNameFormat meshNameFormat = ShapeNameFormat::Product;
        bool embedTextures = true;    // Only applicable if `format` == Format::Binary
        bool mergeFaces = false;
        bool keepIndices16b = false;  // Only applicable if 'mergeFaces' == true
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    class Properties;
    Parameters m_params;
    DocumentPtr m_document;
    TDF_LabelSequence m_seqRootLabel;
};

} // namespace Mayo::IO
