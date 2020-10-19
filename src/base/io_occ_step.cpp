/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_step.h"
#include "io_occ_caf.h"
#include "occ_static_variables_rollback.h"
#include "property_builtins.h"
#include "property_enumeration.h"
#include "task_progress.h"

#include <STEPCAFControl_Controller.hxx>

namespace Mayo {
namespace IO {

namespace {

struct StepReaderParameters : public PropertyGroup {
    StepReaderParameters(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup),
          productContext(this, MAYO_TEXT_ID("Mayo::IO::OccStepReader", "productContext"), &enumProductContext),
          assemblyLevel(this, MAYO_TEXT_ID("Mayo::IO::OccStepReader", "assemblyLevel"), &enumAssemblyLevel),
          readShapeAspect(this, MAYO_TEXT_ID("Mayo::IO::OccStepReader", "readShapeAspect"))
    {
    }

    void restoreDefaults() override {
        this->productContext.setValue(int(ProductContext::All));
        this->assemblyLevel.setValue(int(AssemblyLevel::All));
        this->readShapeAspect.setValue(true);
    }

    using ProductContext = OccStepReader::ProductContext;
    inline static const Enumeration enumProductContext = {
        { int(ProductContext::All), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "All") },
        { int(ProductContext::Design), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "Design") },
        { int(ProductContext::Analysis), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "Analysis") }
    };

    using AssemblyLevel = OccStepReader::AssemblyLevel;
    inline static const Enumeration enumAssemblyLevel = {
        { int(AssemblyLevel::All), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "All") },
        { int(AssemblyLevel::Assembly), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "Assembly") },
        { int(AssemblyLevel::Structure), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "Structure") },
        { int(AssemblyLevel::Shape), MAYO_TEXT_ID("Mayo::IO::OccStepReader", "Shape") },
    };

    PropertyEnumeration productContext;
    PropertyEnumeration assemblyLevel;
    PropertyBool readShapeAspect;
};

struct StepWriterParameters : public PropertyGroup {
    StepWriterParameters(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup),
          schema(this, MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "schema"), &enumSchema),
          assemblyMode(this, MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "assemblyMode"), &enumAssemblyMode),
          freeVertexMode(this, MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "freeVertexMode"), &enumFreeVertexMode),
          writePCurves(this, MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "writeParametericCurves"))
    {
    }

    void restoreDefaults() override {
        this->schema.setValue(int(Schema::AP214_CD));
        this->assemblyMode.setValue(int(AssemblyMode::Skip));
        this->freeVertexMode.setValue(int(FreeVertexMode::Compound));
        this->writePCurves.setValue(true);
    }

    using Schema = OccStepWriter::Schema;
    inline static const Enumeration enumSchema = {
        { int(Schema::AP203), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "AP203") },
        { int(Schema::AP214_CD), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "AP214_CD") },
        { int(Schema::AP214_DIS), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "AP214_DIS") },
        { int(Schema::AP214_IS), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "AP214_IS") },
        { int(Schema::AP242_DIS), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "AP242_DIS") }
    };

    using AssemblyMode = OccStepWriter::AssemblyMode;
    inline static const Enumeration enumAssemblyMode = {
        { int(AssemblyMode::Skip), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "Skip") },
        { int(AssemblyMode::Write), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "Write") },
        { int(AssemblyMode::Auto), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "Auto") }
    };

    using FreeVertexMode = OccStepWriter::FreeVertexMode;
    inline static const Enumeration enumFreeVertexMode = {
        { int(FreeVertexMode::Compound), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "Compound") },
        { int(FreeVertexMode::Single), MAYO_TEXT_ID("Mayo::IO::OccStepWriter", "Single") }
    };

    PropertyEnumeration schema;
    PropertyEnumeration assemblyMode;
    PropertyEnumeration freeVertexMode;
    PropertyBool writePCurves;
};

const char Key_writeStepSchema[] = "write.step.schema";
const char Key_writeStepAssembly[] = "write.step.assembly";
const char Key_writePCurvesMode[] = "write.surfacecurve.mode";
const char Key_writeStepVertexMode[] = "write.step.vertex.mode";

} // namespace

OccStepReader::OccStepReader()
{
    STEPCAFControl_Controller::Init();
    m_reader.SetColorMode(true);
    m_reader.SetNameMode(true);
    m_reader.SetLayerMode(true);
    m_reader.SetPropsMode(true);
}

bool OccStepReader::readFile(const QString& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    // "read.stepcaf.subshapes.name"
    // "read.stepcaf.codepage"
    return cafReadFile(m_reader, filepath, progress);
}

bool OccStepReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    return cafTransfer(m_reader, doc, progress);
}

std::unique_ptr<PropertyGroup> OccStepReader::createParameters(PropertyGroup* parentGroup)
{
    return std::make_unique<StepReaderParameters>(parentGroup);
}

void OccStepReader::applyParameters(const PropertyGroup *params)
{
    auto ptr = dynamic_cast<const StepWriterParameters*>(params);
    if (ptr) {
    }
}

OccStepWriter::OccStepWriter()
{
    STEPCAFControl_Controller::Init();
}

bool OccStepWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return cafTransfer(m_writer, appItems, progress);
}

bool OccStepWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    const IFSelect_ReturnStatus err = m_writer.Write(filepath.toLocal8Bit().constData());
    progress->setValue(100);
    return err == IFSelect_RetDone;
}

std::unique_ptr<PropertyGroup> OccStepWriter::createParameters(PropertyGroup* parentGroup)
{
    return std::make_unique<StepWriterParameters>(parentGroup);
}

void OccStepWriter::applyParameters(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const StepWriterParameters*>(params);
    if (ptr) {
        this->setSchema(ptr->schema.valueAs<Schema>());
        this->setAssemblyMode(ptr->assemblyMode.valueAs<AssemblyMode>());
        this->setFreeVertexMode(ptr->freeVertexMode.valueAs<FreeVertexMode>());
        this->setParametricCurvesMode(
                    ptr->writePCurves.value() ?
                        ParametricCurvesMode::Write :
                        ParametricCurvesMode::Skip);
    }
}

void OccStepWriter::setSchema(OccStepWriter::Schema schema)
{
    m_schema = schema;
    // NOTE from $OCC_7.4.0_DIR/doc/pdf/user_guides/occt_step.pdf (page 26)
    // For the parameter "write.step.schema" to take effect, method STEPControl_Writer::Model(true)
    // should be called after changing this parameter (corresponding command in DRAW is "newmodel")
    m_writer.ChangeWriter().Model(true);
}

void OccStepWriter::changeStaticVariables(OccStaticVariablesRollback* rollback) const
{
    // TODO handle these parameters
    // "write.step.unit"
    // "write.stepcaf.subshapes.name"

    auto fnOccSchema = [](Schema schema) {
        switch (schema) {
        case Schema::AP203: return 3;
        case Schema::AP214_CD: return 1;
        case Schema::AP214_DIS: return 2;
        case Schema::AP214_IS: return 4;
        case Schema::AP242_DIS: return 5;
        }
        Q_UNREACHABLE();
    };
    auto fnOccAssemblyMode = [](AssemblyMode mode) {
        switch (mode) {
        case AssemblyMode::Skip: return 0;
        case AssemblyMode::Write: return 1;
        case AssemblyMode::Auto: return 2;
        }
        Q_UNREACHABLE();
    };
    auto fnOccPCurvesMode = [](ParametricCurvesMode mode) {
        switch (mode) {
        case ParametricCurvesMode::Skip: return 0;
        case ParametricCurvesMode::Write: return 1;
        }
        Q_UNREACHABLE();
    };
    auto fnOccVertexMode = [](FreeVertexMode mode) {
        switch (mode) {
        case FreeVertexMode::Compound: return 0;
        case FreeVertexMode::Single: return 1;
        }
        Q_UNREACHABLE();
    };

    rollback->change(Key_writeStepSchema, fnOccSchema(m_schema));
    rollback->change(Key_writeStepAssembly, fnOccAssemblyMode(m_assemblyMode));
    rollback->change(Key_writePCurvesMode, fnOccPCurvesMode(m_pcurvesMode));
    rollback->change(Key_writeStepVertexMode, fnOccVertexMode(m_freeVertexMode));
}

} // namespace IO
} // namespace Mayo
