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

namespace Mayo {
namespace IO {

// OpenCascade-based writer for glTF format
// Requires OpenCascade >= v7.5.0
class OccGltfWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> spanAppItem, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    enum class Format { Json, Binary };

    struct Parameters {
        RWMesh_CoordinateSystem coordinatesConverter = RWMesh_CoordinateSystem_glTF;
        RWGltf_WriterTrsfFormat transformationFormat = RWGltf_WriterTrsfFormat_Compact;
        Format format = Format::Binary;
        bool forceExportUV = false;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    class Properties;
    Parameters m_params;
    DocumentPtr m_document;
    TDF_LabelSequence m_seqRootLabel;
};

} // namespace IO
} // namespace Mayo

