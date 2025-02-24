/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/tkernel_utils.h"
#include <NCollection_Vector.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>

#include <type_traits>

namespace Mayo::IO {

class OccStaticVariablesRollback;

// Opencascade-based reader for STEP file format
class OccStepReader : public Reader {
public:
    OccStepReader();
    OccStepReader(const OccStepReader&) = delete; // Not copyable
    OccStepReader& operator=(const OccStepReader&) = delete; // Not copyable
    ~OccStepReader();

    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;

    // Parameters

    enum class ProductContext {
        Design = 2, Analysis = 3, Both = 1
    };

    enum class AssemblyLevel {
        Assembly = 2, Structure = 3, Shape = 4, All = 1
    };

    enum class ShapeRepresentation {
        AdvancedBRep = 2,
        ManifoldSurface = 3,
        GeometricallyBoundedSurface = 4,
        FacettedBRep = 5,
        EdgeBasedWireframe = 6,
        GeometricallyBoundedWireframe = 7,
        All = 1
    };

    // Maps to OpenCascade's Resource_FormatType
    enum class Encoding {
        Shift_JIS, // Shift Japanese Industrial Standards
        EUC,       // (Extended Unix Code) multi-byte encoding primarily for Japanese, Korean, and simplified Chinese
        ANSI,
        GB,        // (Guobiao) encoding for Simplified Chinese
        UTF8,
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        // Windows-native ("ANSI") 8-bit code pages
        CP_1250, // Central European
        CP_1251, // Cyrillic
        CP_1252, // Western European
        CP_1253, // Greek
        CP_1254, // Turkish
        CP_1255, // Hebrew
        CP_1256, // Arabic
        CP_1257, // Baltic
        CP_1258, // Vietnamese
        // ISO8859 8-bit code pages
        ISO_8859_1, // Western European
        ISO_8859_2, // Central European
        ISO_8859_3, // Turkish
        ISO_8859_4, // Northern European
        ISO_8859_5, // Cyrillic
        ISO_8859_6, // Arabic
        ISO_8859_7, // Greek
        ISO_8859_8, // Hebrew
        ISO_8859_9, // Turkish
#endif
    };

    struct Parameters {
        ProductContext productContext = ProductContext::Both;
        AssemblyLevel assemblyLevel = AssemblyLevel::All;
        ShapeRepresentation preferredShapeRepresentation = ShapeRepresentation::All;
        bool readShapeAspect = true;
        bool readSubShapesNames = false;
        Encoding encoding = Encoding::UTF8;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

private:
    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    class Properties;
    STEPCAFControl_Reader* m_reader = nullptr;
    std::aligned_storage_t<sizeof(STEPCAFControl_Reader)> m_readerStorage;
    Parameters m_params;
};

// Opencascade-based writer for STEP file format
class OccStepWriter : public Writer {
public:
    OccStepWriter();
    OccStepWriter(const OccStepWriter&) = delete; // Not copyable
    OccStepWriter& operator=(const OccStepWriter&) = delete; // Not copyable
    ~OccStepWriter();

    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters

    enum class Schema {
        AP203 = 3,
        AP214_CD = 1,
        AP214_DIS = 2,
        AP214_IS = 4,
        AP242_DIS = 5
    };

    enum class AssemblyMode {
        Skip = 0, Write = 1, Auto = 2
    };

    enum class FreeVertexMode {
        Compound = 0, Single = 1
    };

    using LengthUnit = OccCommon::LengthUnit;
    struct Parameters {
        Schema schema = Schema::AP214_IS;
        LengthUnit lengthUnit = LengthUnit::Millimeter;
        AssemblyMode assemblyMode = AssemblyMode::Auto;
        FreeVertexMode freeVertexMode = FreeVertexMode::Compound;
        bool writeParametricCurves = true;
        bool writeSubShapesNames = false;
        std::string headerAuthor;            // utf8
        std::string headerOrganization;      // utf8
        std::string headerOriginatingSystem; // utf8
        std::string headerDescription;       // utf8
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

private:
    void changeStaticVariables(OccStaticVariablesRollback* rollback);

    class Properties;
    STEPCAFControl_Writer* m_writer = nullptr;
    std::aligned_storage_t<sizeof(STEPCAFControl_Writer)> m_writerStorage;
    Parameters m_params;
    Schema m_schemaLastTransfer = Schema::AP214_IS;
};

} // namespace Mayo::IO
