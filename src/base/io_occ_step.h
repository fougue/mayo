/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_reader.h"
#include "io_writer.h"
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
        All, Design, Analysis
    };

    enum class AssemblyLevel {
        All, Assembly, Structure, Shape
    };

    // TODO ...

private:
    STEPCAFControl_Reader m_reader;
};

// Opencascade-based reader for STEP file format
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
    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    STEPCAFControl_Writer m_writer;
    Schema m_schema = Schema::AP214_CD;
    AssemblyMode m_assemblyMode = AssemblyMode::Skip;
    FreeVertexMode m_freeVertexMode = FreeVertexMode::Compound;
    ParametricCurvesMode m_pcurvesMode = ParametricCurvesMode::Write;
};

} // namespace IO
} // namespace Mayo
