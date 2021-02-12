/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_gltf_writer.h"

#include "../base/application_item.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/enumeration_fromenum.h"
#include "../base/string_utils.h"
#include "../base/text_id.h"
#include "io_occ_common.h"

#include <RWGltf_CafWriter.hxx>

namespace Mayo {
namespace IO {

class OccGltfWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccGltfWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->coordinatesConverter.setDescription(
                    textIdTr("Coordinate system transformation from OpenCascade to glTF"));
        this->transformationFormat.setDescription(
                    textIdTr("Preferred transformation format for writing into glTF file"));
        this->forceExportUV.setDescription(
                    textIdTr("Export UV coordinates even if there is no mapped texture"));

        this->transformationFormat.mutableEnumeration().chopPrefix("RWGltf_WriterTrsfFormat_");
        this->transformationFormat.setDescriptions({
                    { RWGltf_WriterTrsfFormat_Compact, textIdTr("Automatically choose most compact "
                      "representation between Mat4 and TRS") },
                    { RWGltf_WriterTrsfFormat_Mat4, textIdTr("4x4 transformation matrix") },
                    { RWGltf_WriterTrsfFormat_TRS, textIdTr("Transformation decomposed into Translation "
                      "vector, Rotation quaternion and Scale factor(T * R * S)") }
        });
    }

    void restoreDefaults() override {
        const Parameters defaults;
        this->coordinatesConverter.setValue(defaults.coordinatesConverter);
        this->transformationFormat.setValue(defaults.transformationFormat);
        this->format.setValue(defaults.format);
        this->forceExportUV.setValue(defaults.forceExportUV);
    }

    PropertyEnum<RWMesh_CoordinateSystem> coordinatesConverter{ this, textId("coordinatesConverter") };
    PropertyEnum<RWGltf_WriterTrsfFormat> transformationFormat{ this, textId("transformationFormat") };
    PropertyEnum<Format> format{ this, textId("format") };
    PropertyBool forceExportUV{ this, textId("forceExportUV") };
};

bool OccGltfWriter::transfer(Span<const ApplicationItem> spanAppItem, TaskProgress*)
{
    m_document.Nullify();
    m_seqRootLabel.Clear();
    for (const ApplicationItem& appItem : spanAppItem) {
        if (appItem.isDocument() && m_document.IsNull()) {
            m_document = appItem.document();
        }
        else if (appItem.isDocumentTreeNode()) {
            if (m_document.IsNull())
                m_document = appItem.document();

            if (appItem.document().get() == m_document.get())
                m_seqRootLabel.Append(appItem.documentTreeNode().label());
        }
    }

    if (!m_document)
        return false;

    return true;
}

bool OccGltfWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    if (!m_document)
        return false;

    Handle_Message_ProgressIndicator occProgress = new OccProgressIndicator(progress);
    const bool isBinary = m_params.format == Format::Binary;
    RWGltf_CafWriter writer(StringUtils::toUtf8<TCollection_AsciiString>(filepath), isBinary);
    const TColStd_IndexedDataMapOfStringString fileInfo;
    if (m_seqRootLabel.IsEmpty())
        return writer.Perform(m_document, fileInfo, occProgress->Start());
    else
        return writer.Perform(m_document, m_seqRootLabel, nullptr, fileInfo, occProgress->Start());
}

std::unique_ptr<PropertyGroup> OccGltfWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccGltfWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.coordinatesConverter = ptr->coordinatesConverter;
        m_params.forceExportUV = ptr->forceExportUV;
        m_params.format = ptr->format;
        m_params.transformationFormat = ptr->transformationFormat;
    }
}

} // namespace IO
} // namespace Mayo
