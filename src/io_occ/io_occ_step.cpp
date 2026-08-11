/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_step.h"
#include "io_occ_caf.h"
#include "../base/messenger.h"
#include "../base/meta_enum.h"
#include "../base/occ_handle.h"
#include "../base/occ_static_variables_rollback.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/string_conv.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"
#include "../base/enumeration_fromenum.h"

#include <APIHeaderSection_MakeHeader.hxx>
#include <Interface_Static.hxx>
#include <Interface_Version.hxx>
#include <STEPCAFControl_Controller.hxx>
#include <fmt/format.h>
#include <stdexcept>

namespace Mayo::IO {

OccStepReader::Parameters::Parameters()
{
    this->restoreDefaults();

    this->productContext.setDescription(
        textIdTr("When reading AP 209 STEP files, allows selecting either only `design` "
                 "or `analysis`, or both types of products for translation\n"
                 "Note that in AP 203 and AP214 files all products should be marked as "
                 "`design`, so if this mode is set to `analysis`, nothing will be read")
    );

    this->assemblyLevel.setDescription(
        textIdTr("Specifies which data should be read for the products found in the STEP file")
    );

    this->preferredShapeRepresentation.setDescription(
        textIdTr("Specifies preferred type of representation of the shape of the product, in "
                 "case if a STEP file contains more than one representation (i.e. multiple "
                 "`PRODUCT_DEFINITION_SHAPE` entities) for a single product")
    );

    this->readShapeAspect.setDescription(
        textIdTr("Defines whether shapes associated with the `PRODUCT_DEFINITION_SHAPE` entity "
                 "of the product via `SHAPE_ASPECT` should be translated.\n"
                 "This kind of association was used for the representation of hybrid models (i.e. models "
                 "whose shape is composed of different types of representations) in AP 203 files "
                 "before 1998, but it is also used to associate auxiliary information with the "
                 "sub-shapes of the part. Though STEP translator tries to recognize such cases "
                 "correctly, this parameter may be useful to avoid unconditionally translation "
                 "of shapes associated via `SHAPE_ASPECT` entities.")
    );

    this->readSubShapesNames.setDescription(
        textIdTr("Indicates whether to read sub-shape names from 'Name' attributes of "
                 "STEP Representation Items")
    );

    this->productContext.setDescriptions({
        {
            ProductContext::Design,
            textIdTr("Translate only products that have `PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `design`")
        },
        {
            ProductContext::Analysis,
            textIdTr("Translate only products that have `PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `analysis`")
        },
        {
            ProductContext::Both, textIdTr("Translates all products")
        }
    });

    this->assemblyLevel.setDescriptions({
        {
            AssemblyLevel::Assembly,
            textIdTr("Translate the assembly structure and shapes associated with parts only(not with sub-assemblies)")
        },
        {
            AssemblyLevel::Structure,
            textIdTr("Translate only the assembly structure without shapes(a structure of empty compounds). "
                     "This mode can be useful as an intermediate step in applications requiring specialized processing of assembly parts")
        },
        {
            AssemblyLevel::Shape,
            textIdTr("Translate only shapes associated with the product, ignoring the assembly structure (if any). "
                     "This can be useful to translate only a shape associated with specific product, as a complement to assembly mode")
        },
        {
            AssemblyLevel::All,
            textIdTr("Translate both the assembly structure and all associated shapes. "
                     "If both shape and sub-assemblies are associated with the same product, all of them are read and put in a single compound")
        }
    });

    this->preferredShapeRepresentation.addDescription(
        ShapeRepresentation::All,
        textIdTr("Translate all representations(if more than one, put in compound)")
    );

    this->encoding.setDescriptions({
        { Encoding::Shift_JIS, textIdTr("Shift Japanese Industrial Standards") },
        { Encoding::EUC, textIdTr("EUC(Extended Unix Code), multi-byte encoding primarily for Japanese, Korean, and simplified Chinese") },
        { Encoding::GB, textIdTr("GB(Guobiao) encoding for Simplified Chinese") }
    });
}

void OccStepReader::Parameters::restoreDefaults()
{
    this->productContext.setValue(ProductContext::Both);
    this->assemblyLevel.setValue(AssemblyLevel::All);
    this->preferredShapeRepresentation.setValue(ShapeRepresentation::All);
    this->readShapeAspect.setValue(true);
    this->readSubShapesNames.setValue(false);
    this->encoding.setValue(Encoding::UTF8);
}

OccStepReader::OccStepReader()
{
    MayoIO_CafGlobalScopedLock(cafLock);
    m_reader = new(&m_readerStorage) STEPCAFControl_Reader();
    STEPCAFControl_Controller::Init();
    m_reader->SetColorMode(true);
    m_reader->SetNameMode(true);
    m_reader->SetLayerMode(true);
    m_reader->SetPropsMode(true);
    m_reader->SetGDTMode(true);
    m_reader->SetMatMode(true);
    m_reader->SetViewMode(true);
}

OccStepReader::~OccStepReader()
{
    m_reader->~STEPCAFControl_Reader();
}

bool OccStepReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafReadFile(*m_reader, filepath, progress);
}

NCollection_Sequence<TDF_Label> OccStepReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafTransfer(*m_reader, doc, progress);
}

void OccStepReader::changeStaticVariables(OccStaticVariablesRollback* rollback) const
{
    auto fnOccEncoding = [](Encoding code) {
        switch (code) {
        case Encoding::Shift_JIS: return "SJIS";
        case Encoding::EUC: return "EUC";
        case Encoding::ANSI: return "ANSI";
        case Encoding::GB: return "GB";
        case Encoding::UTF8: return "UTF8";
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        // Windows-native ("ANSI") 8-bit code pages
        case Encoding::CP_1250: return "CP1250";
        case Encoding::CP_1251: return "CP1251";
        case Encoding::CP_1252: return "CP1252";
        case Encoding::CP_1253: return "CP1253";
        case Encoding::CP_1254: return "CP1254";
        case Encoding::CP_1255: return "CP1255";
        case Encoding::CP_1256: return "CP1256";
        case Encoding::CP_1257: return "CP1257";
        case Encoding::CP_1258: return "CP1258";
        // ISO8859 8-bit code pages
        case Encoding::ISO_8859_1: return "iso8859-1";
        case Encoding::ISO_8859_2: return "iso8859-2";
        case Encoding::ISO_8859_3: return "iso8859-3";
        case Encoding::ISO_8859_4: return "iso8859-4";
        case Encoding::ISO_8859_5: return "iso8859-5";
        case Encoding::ISO_8859_6: return "iso8859-6";
        case Encoding::ISO_8859_7: return "iso8859-7";
        case Encoding::ISO_8859_8: return "iso8859-8";
        case Encoding::ISO_8859_9: return "iso8859-9";
#endif
        }
        throw std::invalid_argument(fmt::format("{} isn't supported", MetaEnum::name(code)));
    };

    const char strKeyReadStepCodePage[] =
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        "read.step.codepage";
#else
        "read.stepcaf.codepage";
#endif

    rollback->change("read.step.product.context", static_cast<int>(m_params.productContext.value()));
    rollback->change("read.step.assembly.level", static_cast<int>(m_params.assemblyLevel.value()));
    rollback->change("read.step.shape.repr", static_cast<int>(m_params.preferredShapeRepresentation.value()));
    rollback->change("read.step.shape.aspect", m_params.readShapeAspect.value() ? 1 : 0);
    rollback->change("read.stepcaf.subshapes.name", m_params.readSubShapesNames.value() ? 1 : 0);
    rollback->change(strKeyReadStepCodePage, fnOccEncoding(m_params.encoding.value()));
}

OccStepWriter::Parameters::Parameters()
{
    this->restoreDefaults();

    this->schema.setDescription(textIdTr("Version of schema used for the output STEP file"));

    this->lengthUnit.setDescription(
        textIdTr("Defines a unit in which the STEP file should be written. If set to "
                 "unit other than millimeter, the model is converted to these units "
                 "during the translation")
    );

    this->freeVertexMode.setDescription(
        textIdTr("Parameter to write all free vertices in one SDR (name and style of "
                 "vertex are lost) or each vertex in its own SDR (name and style of "
                 "vertex are exported)")
        );
    this->freeVertexMode.setDescriptions({
        {
            FreeVertexMode::Compound,
            textIdTr("All free vertices are united into one compound and exported in one shape definition "
                     "representation (vertex name and style are lost)")
        },
        {
            FreeVertexMode::Single,
            textIdTr("Each vertex is exported in its own `SHAPE DEFINITION REPRESENTATION` (vertex "
                     "name and style are not lost, but the STEP file size increases)")
        },
    });

    this->writePCurves.setDescription(
        textIdTr("Indicates whether parametric curves (curves in parametric space of surface) should be "
                 "written into the STEP file.\n"
                 "It can be disabled in order to minimize the size of the resulting file.")
    );

    this->writeSubShapesNames.setDescription(
        textIdTr("Indicates whether to write sub-shape names to 'Name' attributes of "
                 "STEP Representation Items")
    );

    this->headerAuthor.setDescription(textIdTr("Author attribute in STEP header"));
    this->headerOrganization.setDescription(textIdTr("Organization(of author) attribute in STEP header"));
    this->headerOriginatingSystem.setDescription(textIdTr("Originating system attribute in STEP header"));
    this->headerDescription.setDescription(textIdTr("Description attribute in STEP header"));
}

void OccStepWriter::Parameters::restoreDefaults()
{
    this->schema.setValue(Schema::AP214_IS);
    this->lengthUnit.setValue(LengthUnit::Millimeter);
    this->assemblyMode.setValue(AssemblyMode::Auto);
    this->freeVertexMode.setValue(FreeVertexMode::Compound);
    this->writePCurves.setValue(true);
    this->writeSubShapesNames.setValue(false);

    this->headerAuthor.setValue({});
    this->headerOrganization.setValue({});
    this->headerOriginatingSystem.setValue(XSTEP_SYSTEM_VERSION);
    this->headerDescription.setValue("OpenCascade Model");
}

OccStepWriter::OccStepWriter()
{
    MayoIO_CafGlobalScopedLock(cafLock);
    m_writer = new(&m_writerStorage) STEPCAFControl_Writer();
    STEPCAFControl_Controller::Init();
    m_writer->SetColorMode(true);
    m_writer->SetNameMode(true);
    m_writer->SetLayerMode(true);
    m_writer->SetPropsMode(true);
    m_writer->SetDimTolMode(true);
    m_writer->SetMaterialMode(true);
}

OccStepWriter::~OccStepWriter()
{
    m_writer->~STEPCAFControl_Writer();
}

bool OccStepWriter::transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    if (m_params.schema != m_schemaLastTransfer) {
        // NOTE from $OCC_7.4.0_DIR/doc/pdf/user_guides/occt_step.pdf (page 26)
        // For the parameter "write.step.schema" to take effect, method STEPControl_Writer::Model(true)
        // should be called after changing this parameter (corresponding command in DRAW is "newmodel")
        m_writer->ChangeWriter().Model(true);
        m_schemaLastTransfer = m_params.schema;
    }

    return Private::cafTransfer(*m_writer, appItems, progress);
}

bool OccStepWriter::writeFile(const FilePath& filepath, TaskProgress* /*progress*/)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);

    APIHeaderSection_MakeHeader makeHeader(m_writer->ChangeWriter().Model());
    makeHeader.SetAuthorValue(1, to_OccHandleHAsciiString(m_params.headerAuthor.value()));
    makeHeader.SetOrganizationValue(1, to_OccHandleHAsciiString(m_params.headerOrganization.value()));
    makeHeader.SetOriginatingSystem(to_OccHandleHAsciiString(m_params.headerOriginatingSystem.value()));
    makeHeader.SetDescriptionValue(1, to_OccHandleHAsciiString(m_params.headerDescription.value()));
    const IFSelect_ReturnStatus err = m_writer->Write(filepath.u8string().c_str());
    return err == IFSelect_RetDone;
}

void OccStepWriter::changeStaticVariables(OccStaticVariablesRollback* rollback) const
{
    rollback->change("write.step.schema", static_cast<int>(m_params.schema.value()));
    rollback->change("write.step.unit", OccCommon::toCafString(m_params.lengthUnit.value()));
    rollback->change("write.step.assembly", static_cast<int>(m_params.assemblyMode.value()));
    rollback->change("write.step.vertex.mode", static_cast<int>(m_params.freeVertexMode.value()));
    rollback->change("write.surfacecurve.mode", m_params.writePCurves.value() ? 1 : 0);
    rollback->change("write.stepcaf.subshapes.name", m_params.writeSubShapesNames.value() ? 1 : 0);
}

} // namespace Mayo::IO
