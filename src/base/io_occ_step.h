/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_reader.h"
#include "io_writer.h"
#include <QtCore/QObject>
#include <NCollection_Vector.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>

namespace Mayo {
namespace IO {

class OccStaticVariablesRollback;

// Opencascade-based reader for STEP file format
class OccStepReader : public Reader {
public:
    OccStepReader();

    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createParameters(PropertyGroup* parentGroup);
    void applyParameters(const PropertyGroup* params) override;

    // Parameters

    enum class ProductContext {
        Design, Analysis, Both
    };

    enum class AssemblyLevel {
        Assembly, Structure, Shape, All
    };

    enum class ShapeRepresentation {
        AdvancedBRep,
        ManifoldSurface,
        GeometricallyBoundedSurface,
        FacettedBRep,
        EdgeBasedWireframe,
        GeometricallyBoundedWireframe,
        All
    };

    // Maps to OpenCascade's Resource_FormatType
    enum class Encoding {
        Shift_JIS, // Shift Japanese Industrial Standards
        EUC, // (Extended Unix Code) multi-byte encoding primarily for Japanese, Korean, and simplified Chinese
        ANSI,
        GB, // (Guobiao) encoding for Simplified Chinese
        UTF8
    };

    struct Options {
        ProductContext productContext = ProductContext::Both;
        AssemblyLevel assemblyLevel = AssemblyLevel::All;
        ShapeRepresentation preferredShapeRepresentation = ShapeRepresentation::All;
        bool readShapeAspect = true;
        Encoding encoding = Encoding::UTF8;
    };
    const Options& options() const { return m_options; }
    void setOptions(const Options& options) { m_options = options; }

private:
    Q_GADGET
    Q_ENUM(Encoding)

    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    class Parameters;
    STEPCAFControl_Reader m_reader;
    Options m_options;
};

// Opencascade-based writer for STEP file format
class OccStepWriter : public Writer {
public:
    OccStepWriter();

    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createParameters(PropertyGroup* parentGroup);
    void applyParameters(const PropertyGroup* params) override;

    // Parameters

    enum class Schema {
        AP203,
        AP214_CD,
        AP214_DIS,
        AP214_IS,
        AP242_DIS
    };

    enum class AssemblyMode {
        Skip, Write, Auto
    };

    enum class FreeVertexMode {
        Compound, Single
    };

    enum class ParametricCurvesMode {
        Skip, Write
    };

    Schema schema() const { return m_schema; }
    void setSchema(Schema schema);

    AssemblyMode assemblyMode() const { return m_assemblyMode; }
    void setAssemblyMode(AssemblyMode mode) { m_assemblyMode = mode; }

    FreeVertexMode freeVertexMode() const { return m_freeVertexMode; }
    void setFreeVertexMode(FreeVertexMode mode) { m_freeVertexMode = mode; }

    ParametricCurvesMode parametricCurvesMode() const { return m_pcurvesMode; }
    void setParametricCurvesMode(ParametricCurvesMode mode) { m_pcurvesMode = mode; }

private:
    Q_GADGET
    Q_ENUM(Schema)
    Q_ENUM(AssemblyMode)
    Q_ENUM(FreeVertexMode)
    Q_ENUM(ParametricCurvesMode)

    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    class Parameters;
    STEPCAFControl_Writer m_writer;
    Schema m_schema = Schema::AP214_CD;
    AssemblyMode m_assemblyMode = AssemblyMode::Skip;
    FreeVertexMode m_freeVertexMode = FreeVertexMode::Compound;
    ParametricCurvesMode m_pcurvesMode = ParametricCurvesMode::Write;
};

} // namespace IO
} // namespace Mayo
