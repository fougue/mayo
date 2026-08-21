/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "../base/property_binding.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/text_id.h"
#include "io_occ_common.h"

#include <Standard_Version.hxx>

#if OCC_VERSION_HEX >= 0x070500
#  include "io_occ_gltf_writer.h"

namespace Mayo::IO {

namespace {

struct OccGltfWriterMeta {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccGltfWriterMeta)
public:
    // Enums
    inline static const auto enumTrsfFormat =
        Enumeration::fromType<RWGltf_WriterTrsfFormat>(textIdContext())
        .chopPrefix("RWGltf_WriterTrsfFormat_")
        .setDescription(
            RWGltf_WriterTrsfFormat_Compact,
                textId("Automatically choose most compact representation between Mat4 and TRS")
        )
        .setDescription(
            RWGltf_WriterTrsfFormat_Mat4, textId("4x4 transformation matrix")
        )
        .setDescription(
            RWGltf_WriterTrsfFormat_TRS,
            textId("Transformation decomposed into Translation vector, Rotation quaternion and Scale factor(T * R * S)")
        )
    ;

    inline static const auto enumFormat =
        Enumeration::fromType<OccGltfWriter::Format>(textIdContext());

    inline static const auto enumShapeNameFormat =
        Enumeration::fromType<OccGltfWriter::ShapeNameFormat>(textIdContext());

    // Metas
    inline static const auto inputCoordSystem =
        PropertyEnumMeta{ textId("inputCoordinateSystem"),  OccCommon::enum_RWMesh_CoordinateSystem() }
        .setDescription(textId("Source coordinate system transformation"))
    ;
    inline static const auto outputCoordSystem =
        PropertyEnumMeta{ textId("outputCoordinateSystem"), OccCommon::enum_RWMesh_CoordinateSystem() }
        .setDescription(textId("Target coordinate system transformation"))
    ;
    inline static const auto trsfFormat =
        PropertyEnumMeta{ textId("transformationFormat"), enumTrsfFormat }
        .setDescription(textId("Preferred transformation format for writing into glTF file"))
    ;

    inline static const auto format = PropertyEnumMeta{textId("format"), enumFormat};

    inline static const auto forceExportUV =
        PropertyMeta{textId("forceExportUV")}
        .setDescription(textId("Export UV coordinates even if there is no mapped texture"))
    ;
    inline static const auto nodeNameFormat =
        PropertyEnumMeta{ textId("nodeNameFormat"), enumShapeNameFormat }
        .setDescription(textId("Name format for exporting nodes"))
    ;
    inline static const auto meshNameFormat =
        PropertyEnumMeta{ textId("meshNameFormat"), enumShapeNameFormat }
        .setDescription(textId("Name format for exporting meshes"))
    ;
    inline static const auto mergeFaces =
        PropertyMeta{textId("mergeFaces")}
        .setDescription(textId(
            "Merge faces within a single part.\n\n"
            "May reduce JSON size thanks to smaller number of primitive arrays"
        ))
    ;
    inline static const auto embedTextures =
        PropertyMeta{textId("embedTextures")}
        .setDescription(fmt::format(textIdTr(
            "Write image textures into target file.\n\n"
            "If set to `false` then texture images will be written as separate files.\n\n"
            "Applicable only if option `{0}` is set to `{1}`"
            ),
            format.name().tr(), enumFormat.findNameByValue(OccGltfWriter::Format::Binary).tr()
        ))
    ;
    inline static const auto keepIndices16b =
        PropertyMeta{textId("keepIndices16b")}
        .setDescription(fmt::format(textIdTr(
            "Prefer keeping 16-bit indexes while merging face.\n\n"
            "May reduce binary data size thanks to smaller triangle indexes.\n\n"
            "Applicable only if option `{}` is on"
            ),
            mergeFaces.name().tr()
        ))
    ;
};

struct OccGltfWriterProperties :
        public WriterProperties,
        private PropertyBindingGroup<OccGltfWriter::Parameters>
{
    template<typename PropertyType> using Bind = BoundProperty<PropertyType, OccGltfWriter::Parameters>;
    using Params = OccGltfWriter::Parameters;
    using Meta = OccGltfWriterMeta;
    using BindingGroup = PropertyBindingGroup<Params>;

    Bind<PropertyEnum> inputCoordSystem{ this, Meta::inputCoordSystem, &Params::inputCoordinateSystem };
    Bind<PropertyEnum> outputCoordSystem{ this, Meta::outputCoordSystem, &Params::outputCoordinateSystem };
    Bind<PropertyEnum> trsfFormat{ this, Meta::trsfFormat, &Params::transformationFormat };
    Bind<PropertyEnum> format{ this, Meta::format, &Params::format };
    Bind<PropertyBool> forceExportUV{ this, Meta::forceExportUV, &Params::forceExportUV };
    Bind<PropertyEnum> nodeNameFormat{ this, Meta::nodeNameFormat, &Params::nodeNameFormat };
    Bind<PropertyEnum> meshNameFormat{ this, Meta::meshNameFormat, &Params::meshNameFormat };
    Bind<PropertyBool> embedTextures{ this, Meta::embedTextures, &Params::embedTextures };
    Bind<PropertyBool> mergeFaces{ this, Meta::mergeFaces, &Params::mergeFaces };
    Bind<PropertyBool> keepIndices16b{ this, Meta::keepIndices16b, &Params::keepIndices16b };

    bool saveTo(Writer& writer) const override
    {
        auto ptr = dynamic_cast<OccGltfWriter*>(&writer);
        return ptr ? BindingGroup::saveTo(ptr->parameters()) : false;
    }

    bool loadFrom(const Writer& writer) override
    {
        auto ptr = dynamic_cast<const OccGltfWriter*>(&writer);
        return ptr ? BindingGroup::loadFrom(ptr->constParameters()) : false;
    }

    void restoreDefaults() override
    {
        BindingGroup::loadFrom(OccGltfWriter::Parameters{});
    }

protected:
    void onPropertyChanged(Property* prop) override
    {
        if (prop == &this->format)
            this->embedTextures.setEnabled(this->format.value() == OccGltfWriter::Format::Binary);
        else if (prop == &this->mergeFaces)
            this->keepIndices16b.setEnabled(this->mergeFaces);

        PropertyGroup::onPropertyChanged(prop);
    }
};

} // namespace

std::unique_ptr<PropertyGroup> makeOccGltfWriterProperties()
{
    return std::make_unique<OccGltfWriterProperties>();
}

} // namespace Mayo::IO

#endif
