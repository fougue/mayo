/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"

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

    struct Parameters : public PropertyGroup {
        PropertyEnum<RWMesh_CoordinateSystem> inputCoordinateSystem{ this, textId("inputCoordinateSystem") };
        PropertyEnum<RWMesh_CoordinateSystem> outputCoordinateSystem{ this, textId("outputCoordinateSystem") };
        PropertyEnum<RWGltf_WriterTrsfFormat> transformationFormat{ this, textId("transformationFormat") };
        PropertyEnum<Format> format{ this, textId("format") };
        PropertyBool forceExportUV{ this, textId("forceExportUV") };
        PropertyEnum<OccGltfWriter::ShapeNameFormat> nodeNameFormat{ this, textId("nodeNameFormat") };
        PropertyEnum<OccGltfWriter::ShapeNameFormat> meshNameFormat{ this, textId("meshNameFormat") };
        PropertyBool embedTextures{ this, textId("embedTextures") };   // Only applicable if `format` == Format::Binary
        PropertyBool mergeFaces{ this, textId("mergeFaces") };
        PropertyBool keepIndices16b{ this, textId("keepIndices16b") }; // Only applicable if 'mergeFaces' == true

        Parameters();
        void restoreDefaults() override;

    protected:
        void onPropertyChanged(Property* prop) override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccGltfWriter)

    Parameters m_params;
    DocumentPtr m_document;
    NCollection_Sequence<TDF_Label> m_seqRootLabel;
};

} // namespace Mayo::IO
