/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_step.h"
#include "io_occ_caf.h"
#include "../base/occ_static_variables_rollback.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/string_utils.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"
#include "../base/enumeration_fromenum.h"

#include <APIHeaderSection_MakeHeader.hxx>
#include <Interface_Static.hxx>
#include <Interface_Version.hxx>
#include <STEPCAFControl_Controller.hxx>

namespace Mayo {
namespace IO {

class OccStepReader::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccStepReader::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->productContext.setDescription(
                    textIdTr("When reading AP 209 STEP files, allows selecting either only `design` "
                             "or `analysis`, or both types of products for translation\n"
                             "Note that in AP 203 and AP214 files all products should be marked as "
                             "`design`, so if this mode is set to `analysis`, nothing will be read"));

        this->assemblyLevel.setDescription(
                    textIdTr("Specifies which data should be read for the products found in the STEP file"));

        this->preferredShapeRepresentation.setDescription(
                    textIdTr("Specifies preferred type of representation of the shape of the product, in "
                             "case if a STEP file contains more than one representation (i.e. multiple "
                             "`PRODUCT_DEFINITION_SHAPE` entities) for a single product"));

        this->readShapeAspect.setDescription(
                    textIdTr("Defines whether shapes associated with the `PRODUCT_DEFINITION_SHAPE` entity "
                             "of the product via `SHAPE_ASPECT` should be translated.\n"
                             "This kind of association was used for the representation of hybrid models (i.e. models "
                             "whose shape is composed of different types of representations) in AP 203 files "
                             "before 1998, but it is also used to associate auxiliary information with the "
                             "sub-shapes of the part. Though STEP translator tries to recognize such cases "
                             "correctly, this parameter may be useful to avoid unconditionally translation "
                             "of shapes associated via `SHAPE_ASPECT` entities."));

        this->readSubShapesNames.setDescription(
                    textIdTr("Indicates whether to read sub-shape names from 'Name' attributes of "
                             "STEP Representation Items"));

        this->productContext.setDescriptions({
                    { ProductContext::Design, textIdTr("Translate only products that have "
                      "`PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `design`")
                    },
                    { ProductContext::Analysis, textIdTr("Translate only products that have "
                      "`PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `analysis`")
                    },
                    { ProductContext::Both, textIdTr("Translates all products") }
        });

        this->assemblyLevel.setDescriptions({
                    { AssemblyLevel::Assembly, textIdTr("Translate the assembly structure and shapes "
                      "associated with parts only(not with sub-assemblies)")
                    },
                    { AssemblyLevel::Structure, textIdTr("Translate only the assembly structure "
                      "without shapes(a structure of empty compounds). This mode can be useful as "
                      "an intermediate step in applications requiring specialized processing of assembly parts")
                    },
                    { AssemblyLevel::Shape, textIdTr("Translate only shapes associated with the "
                      "product, ignoring the assembly structure (if any). This can be useful to "
                      "translate only a shape associated with specific product, as a complement to assembly mode")
                    },
                    { AssemblyLevel::All, textIdTr("Translate both the assembly structure and all "
                      "associated shapes. If both shape and sub-assemblies are associated with the "
                      "same product, all of them are read and put in a single compound")
                    }
        });

        this->preferredShapeRepresentation.addDescription(
                    ShapeRepresentation::All,
                    textIdTr("Translate all representations(if more than one, put in compound)"));

        this->encoding.setDescriptions({
                    { Encoding::Shift_JIS, textIdTr("Shift Japanese Industrial Standards") },
                    { Encoding::EUC, textIdTr("EUC(Extended Unix Code), multi-byte encoding primarily "
                      "for Japanese, Korean, and simplified Chinese") },
                    { Encoding::GB, textIdTr("GB(Guobiao) encoding for Simplified Chinese") }
        });
    }

    void restoreDefaults() override {
        const OccStepReader::Parameters params;
        this->productContext.setValue(params.productContext);
        this->assemblyLevel.setValue(params.assemblyLevel);
        this->readShapeAspect.setValue(params.readShapeAspect);
        this->readSubShapesNames.setValue(params.readSubShapesNames);
        this->encoding.setValue(params.encoding);
    }

    PropertyEnum<ProductContext> productContext{ this, textId("productContext") };
    PropertyEnum<AssemblyLevel> assemblyLevel{ this, textId("assemblyLevel") };
    PropertyEnum<ShapeRepresentation> preferredShapeRepresentation{ this, textId("preferredShapeRepresentation") };
    PropertyBool readShapeAspect{ this, textId("readShapeAspect") };
    PropertyBool readSubShapesNames{ this, textId("readSubShapesNames") };
    PropertyEnum<Encoding> encoding{ this, textId("encoding") };
};

OccStepReader::OccStepReader()
{
    STEPCAFControl_Controller::Init();
    m_reader.SetColorMode(true);
    m_reader.SetNameMode(true);
    m_reader.SetLayerMode(true);
    m_reader.SetPropsMode(true);
    m_reader.SetGDTMode(true);
    m_reader.SetMatMode(true);
    m_reader.SetViewMode(true);
}

bool OccStepReader::readFile(const QString& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafReadFile(m_reader, filepath, progress);
}

bool OccStepReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafTransfer(m_reader, doc, progress);
}

std::unique_ptr<PropertyGroup> OccStepReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccStepReader::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.productContext = ptr->productContext;
        m_params.assemblyLevel = ptr->assemblyLevel;
        m_params.preferredShapeRepresentation = ptr->preferredShapeRepresentation;
        m_params.readShapeAspect = ptr->readShapeAspect;
        m_params.readSubShapesNames = ptr->readSubShapesNames;
        m_params.encoding = ptr->encoding;
    }
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
        Q_UNREACHABLE();
    };

    const char strKeyReadStepCodePage[] =
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        "read.step.codepage";
#else
        "read.stepcaf.codepage";
#endif

    rollback->change("read.step.product.context", int(m_params.productContext));
    rollback->change("read.step.assembly.level", int(m_params.assemblyLevel));
    rollback->change("read.step.shape.repr", int(m_params.preferredShapeRepresentation));
    rollback->change("read.step.shape.aspect", int(m_params.readShapeAspect ? 1 : 0));
    rollback->change("read.stepcaf.subshapes.name", int(m_params.readSubShapesNames ? 1 : 0));
    rollback->change(strKeyReadStepCodePage, fnOccEncoding(m_params.encoding));
}

class OccStepWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccStepWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->schema.setDescription(textIdTr("Version of schema used for the output STEP file"));

        this->lengthUnit.setDescription(
                    textIdTr("Defines a unit in which the STEP file should be written. If set to "
                             "unit other than millimeter, the model is converted to these units "
                             "during the translation"));

        this->freeVertexMode.setDescription(
                    textIdTr("Parameter to write all free vertices in one SDR (name and style of "
                             "vertex are lost) or each vertex in its own SDR (name and style of "
                             "vertex are exported)"));
        this->freeVertexMode.setDescriptions({
                    { FreeVertexMode::Compound, textIdTr("All free vertices are united into one "
                      "compound and exported in one shape definition representation (vertex name "
                      "and style are lost)") },
                    { FreeVertexMode::Single, textIdTr("Each vertex is exported in its own "
                      "`SHAPE DEFINITION REPRESENTATION`(vertex name and style are not lost, but "
                      "the STEP file size increases)") },
        });

        this->writePCurves.setDescription(
                    textIdTr("Indicates whether parametric curves (curves in parametric space of surface) should be "
                             "written into the STEP file.\n"
                             "It can be disabled in order to minimize the size of the resulting file."));

        this->writeSubShapesNames.setDescription(
                    textIdTr("Indicates whether to write sub-shape names to 'Name' attributes of "
                             "STEP Representation Items"));

        this->headerAuthor.setDescription(textIdTr("Author attribute in STEP header"));
        this->headerOrganization.setDescription(textIdTr("Organization(of author) attribute in STEP header"));
        this->headerOriginatingSystem.setDescription(textIdTr("Originating system attribute in STEP header"));
        this->headerDescription.setDescription(textIdTr("Description attribute in STEP header"));
    }

    void restoreDefaults() override {
        const OccStepWriter::Parameters params;
        this->schema.setValue(params.schema);
        this->lengthUnit.setValue(params.lengthUnit);
        this->assemblyMode.setValue(params.assemblyMode);
        this->freeVertexMode.setValue(params.freeVertexMode);
        this->writePCurves.setValue(params.writeParametricCurves);
        this->writeSubShapesNames.setValue(params.writeSubShapesNames);

        this->headerAuthor.setValue(QString());
        this->headerOrganization.setValue(QString());
        this->headerOriginatingSystem.setValue(XSTEP_SYSTEM_VERSION);
        this->headerDescription.setValue("OpenCascade Model");
    }

    PropertyEnum<Schema> schema{ this, textId("schema") };
    PropertyEnum<LengthUnit> lengthUnit{ this, textId("lengthUnit") };
    PropertyEnum<AssemblyMode> assemblyMode{ this, textId("assemblyMode") };
    PropertyEnum<FreeVertexMode> freeVertexMode{ this, textId("freeVertexMode") };
    PropertyBool writePCurves{ this, textId("writeParametericCurves") };
    PropertyBool writeSubShapesNames{ this, textId("writeSubShapesNames") };
    PropertyQString headerAuthor{ this, textId("headerAuthor") };
    PropertyQString headerOrganization{ this, textId("headerOrganization") };
    PropertyQString headerOriginatingSystem{ this, textId("headerOriginatingSystem") };
    PropertyQString headerDescription{ this, textId("headerDescription") };
};

OccStepWriter::OccStepWriter()
{
    STEPCAFControl_Controller::Init();
    m_writer.SetColorMode(true);
    m_writer.SetNameMode(true);
    m_writer.SetLayerMode(true);
    m_writer.SetPropsMode(true);
    m_writer.SetDimTolMode(true);
    m_writer.SetMaterialMode(true);
}

bool OccStepWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafTransfer(m_writer, appItems, progress);
}

bool OccStepWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);

    APIHeaderSection_MakeHeader makeHeader(m_writer.ChangeWriter().Model());
    makeHeader.SetAuthorValue(
                1, StringUtils::toUtf8<Handle_TCollection_HAsciiString>(m_params.headerAuthor));
    makeHeader.SetOrganizationValue(
                1, StringUtils::toUtf8<Handle_TCollection_HAsciiString>(m_params.headerOrganization));
    makeHeader.SetOriginatingSystem(
                StringUtils::toUtf8<Handle_TCollection_HAsciiString>(m_params.headerOriginatingSystem));
    makeHeader.SetDescriptionValue(
                1, StringUtils::toUtf8<Handle_TCollection_HAsciiString>(m_params.headerDescription));

    const IFSelect_ReturnStatus err = m_writer.Write(filepath.toUtf8().constData());
    progress->setValue(100);
    return err == IFSelect_RetDone;
}

std::unique_ptr<PropertyGroup> OccStepWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccStepWriter::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.schema = ptr->schema;
        m_params.lengthUnit = ptr->lengthUnit;
        m_params.assemblyMode = ptr->assemblyMode;
        m_params.freeVertexMode = ptr->freeVertexMode;
        m_params.writeParametricCurves = ptr->writePCurves;
        m_params.writeSubShapesNames = ptr->writeSubShapesNames;
        m_params.headerAuthor = ptr->headerAuthor;
        m_params.headerOrganization = ptr->headerOrganization;
        m_params.headerOriginatingSystem = ptr->headerOriginatingSystem;
        m_params.headerDescription = ptr->headerDescription;
    }
}

void OccStepWriter::changeStaticVariables(OccStaticVariablesRollback* rollback)
{
    const int previousSchema = Interface_Static::IVal("write.step.schema");
    rollback->change("write.step.schema", int(m_params.schema));
    if (int(m_params.schema) != previousSchema) {
        // NOTE from $OCC_7.4.0_DIR/doc/pdf/user_guides/occt_step.pdf (page 26)
        // For the parameter "write.step.schema" to take effect, method STEPControl_Writer::Model(true)
        // should be called after changing this parameter (corresponding command in DRAW is "newmodel")
        m_writer.ChangeWriter().Model(true);
    }

    rollback->change("write.step.unit", OccCommon::toCafString(m_params.lengthUnit));
    rollback->change("write.step.assembly", int(m_params.assemblyMode));
    rollback->change("write.step.vertex.mode", int(m_params.freeVertexMode));
    rollback->change("write.surfacecurve.mode", int(m_params.writeParametricCurves ? 1 : 0));
    rollback->change("write.stepcaf.subshapes.name", int(m_params.writeSubShapesNames ? 1 : 0));
}

} // namespace IO
} // namespace Mayo
