/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_gltf_writer.h"

#include "../base/application_item.h"
#include "../base/enumeration_fromenum.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/text_id.h"
#include "io_occ_common.h"

#include <fmt/format.h>
#include <RWGltf_CafWriter.hxx>

namespace Mayo::IO {

class OccGltfWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccGltfWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->inputCoordinateSystem.setDescription(
                    textIdTr("Source coordinate system transformation"));
        this->outputCoordinateSystem.setDescription(
                    textIdTr("Target coordinate system transformation"));
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

        this->format.mutableEnumeration().changeTrContext(OccGltfWriter::Properties::textIdContext());

        this->nodeNameFormat.setDescription(textIdTr("Name format for exporting nodes"));
        this->meshNameFormat.setDescription(textIdTr("Name format for exporting meshes"));
        this->embedTextures.setDescription(
                    fmt::format(textIdTr("Write image textures into target file.\n\n"
                                         "If set to `false` then texture images will be written as separate files.\n\n"
                                         "Applicable only if option `{0}` is set to `{1}`"),
                                this->format.label(),
                                MetaEnum::name<Format>(Format::Binary)
                    )
        );
        this->mergeFaces.setDescription(
                    textIdTr("Merge faces within a single part.\n\n"
                             "May reduce JSON size thanks to smaller number of primitive arrays")
        );
        this->keepIndices16b.setDescription(
                    fmt::format(textIdTr("Prefer keeping 16-bit indexes while merging face.\n\n"
                                         "May reduce binary data size thanks to smaller triangle indexes.\n\n"
                                         "Applicable only if option `{}` is on"),
                                this->mergeFaces.label()
                    )
        );
    }

    void restoreDefaults() override
    {
        const Parameters defaults;
        this->inputCoordinateSystem.setValue(defaults.inputCoordinateSystem);
        this->outputCoordinateSystem.setValue(defaults.outputCoordinateSystem);
        this->transformationFormat.setValue(defaults.transformationFormat);
        this->format.setValue(defaults.format);
        this->forceExportUV.setValue(defaults.forceExportUV);
#if OCC_VERSION_HEX >= 0x070600
        this->nodeNameFormat.setValue(defaults.nodeNameFormat);
        this->meshNameFormat.setValue(defaults.meshNameFormat);
#else
        this->nodeNameFormat.setValue(ShapeNameFormat::Empty);
        this->meshNameFormat.setValue(ShapeNameFormat::Empty);
#endif
        this->embedTextures.setValue(defaults.embedTextures);
        this->mergeFaces.setValue(defaults.mergeFaces);
        this->keepIndices16b.setValue(defaults.keepIndices16b);

        this->embedTextures.setEnabled(this->format == OccGltfWriter::Format::Binary);
        this->keepIndices16b.setEnabled(this->mergeFaces);
    }

    void onPropertyChanged(Property* prop) override
    {
        if (prop == &this->format)
            this->embedTextures.setEnabled(this->format == OccGltfWriter::Format::Binary);
        else if (prop == &this->mergeFaces)
            this->keepIndices16b.setEnabled(this->mergeFaces);

        PropertyGroup::onPropertyChanged(prop);
    }

    PropertyEnum<RWMesh_CoordinateSystem> inputCoordinateSystem{ this, textId("inputCoordinateSystem") };
    PropertyEnum<RWMesh_CoordinateSystem> outputCoordinateSystem{ this, textId("outputCoordinateSystem") };
    PropertyEnum<RWGltf_WriterTrsfFormat> transformationFormat{ this, textId("transformationFormat") };
    PropertyEnum<Format> format{ this, textId("format") };
    PropertyBool forceExportUV{ this, textId("forceExportUV") };
    PropertyEnum<OccGltfWriter::ShapeNameFormat> nodeNameFormat{ this, textId("nodeNameFormat") };
    PropertyEnum<OccGltfWriter::ShapeNameFormat> meshNameFormat{ this, textId("meshNameFormat") };
    PropertyBool embedTextures{ this, textId("embedTextures") };
    PropertyBool mergeFaces{ this, textId("mergeFaces") };
    PropertyBool keepIndices16b{ this, textId("keepIndices16b") };
};

bool OccGltfWriter::transfer(Span<const ApplicationItem> spanAppItem, TaskProgress*)
{
    m_document.Nullify();
    m_seqRootLabel.Clear();
    System::visitUniqueItems(spanAppItem, [=](const ApplicationItem& appItem) {
        if (appItem.isDocument() && m_document.IsNull()) {
            m_document = appItem.document();
        }
        else if (appItem.isDocumentTreeNode()) {
            if (m_document.IsNull())
                m_document = appItem.document();

            if (appItem.document().get() == m_document.get())
                m_seqRootLabel.Append(appItem.documentTreeNode().label());
        }
    });

    if (!m_document)
        return false;

    return true;
}

bool OccGltfWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (!m_document)
        return false;

    auto occProgress = makeOccHandle<OccProgressIndicator>(progress);
    const bool isBinary = m_params.format == Format::Binary;
    RWGltf_CafWriter writer(filepath.u8string().c_str(), isBinary);
    writer.ChangeCoordinateSystemConverter().SetInputCoordinateSystem(m_params.inputCoordinateSystem);
    writer.ChangeCoordinateSystemConverter().SetOutputCoordinateSystem(m_params.outputCoordinateSystem);
#if OCC_VERSION_HEX >= 0x070600
    auto fnToOccNameFormat = [](ShapeNameFormat format) {
        switch (format) {
        case ShapeNameFormat::Empty: return RWMesh_NameFormat_Empty;
        case ShapeNameFormat::Product: return RWMesh_NameFormat_Product;
        case ShapeNameFormat::Instance: return RWMesh_NameFormat_Instance;
        case ShapeNameFormat::InstanceOrProduct: return RWMesh_NameFormat_InstanceOrProduct;
        case ShapeNameFormat::ProductOrInstance: return RWMesh_NameFormat_ProductOrInstance;
        case ShapeNameFormat::ProductAndInstance: return RWMesh_NameFormat_ProductAndInstance;
        }

        return RWMesh_NameFormat_Empty;
    };
    writer.SetNodeNameFormat(fnToOccNameFormat(m_params.nodeNameFormat));
    writer.SetMeshNameFormat(fnToOccNameFormat(m_params.meshNameFormat));
    writer.SetToEmbedTexturesInGlb(m_params.embedTextures);
    writer.SetMergeFaces(m_params.mergeFaces);
    writer.SetSplitIndices16(m_params.keepIndices16b);
#else
    auto fnWarningOptionNA = [](std::string_view optionName) {
        return fmt::format(Properties::textIdTr("Option supported from OpenCascade ≥ v7.6 [option={}, actual version={}]"),
                           optionName,
                           OCC_VERSION_COMPLETE);
    };
    if (m_params.nodeNameFormat != ShapeNameFormat::Empty)
        this->messenger()->emitWarning(fnWarningOptionNA("nodeNameFormat"));

    if (m_params.meshNameFormat != ShapeNameFormat::Empty)
        this->messenger()->emitWarning(fnWarningOptionNA("meshNameFormat"));

    const Parameters defaultParams;
    if (m_params.embedTextures != defaultParams.embedTextures)
        this->messenger()->emitWarning(fnWarningOptionNA("embedTextures"));

    if (m_params.mergeFaces != defaultParams.mergeFaces)
        this->messenger()->emitWarning(fnWarningOptionNA("mergeFaces"));

    if (m_params.keepIndices16b != defaultParams.keepIndices16b)
        this->messenger()->emitWarning(fnWarningOptionNA("keepIndices16b"));

#endif
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
        m_params.inputCoordinateSystem = ptr->inputCoordinateSystem;
        m_params.outputCoordinateSystem = ptr->outputCoordinateSystem;
        m_params.forceExportUV = ptr->forceExportUV;
        m_params.format = ptr->format;
        m_params.transformationFormat = ptr->transformationFormat;
        m_params.nodeNameFormat = ptr->nodeNameFormat;
        m_params.meshNameFormat = ptr->meshNameFormat;
        m_params.embedTextures = ptr->embedTextures;
        m_params.mergeFaces = ptr->mergeFaces;
        m_params.keepIndices16b = ptr->keepIndices16b;
    }
}

} // namespace Mayo::IO
