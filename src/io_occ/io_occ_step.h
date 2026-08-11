/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/tkernel_utils.h"

#include <Standard_Version.hxx>
#if OCC_VERSION_HEX == 0x070400
#  include <NCollection_Vector.hxx> // Needed by STEPCAFControl_Writer.hxx
#endif
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
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;

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

    struct Parameters : public PropertyGroup {
        PropertyEnum<ProductContext> productContext{ this, textId("productContext") };
        PropertyEnum<AssemblyLevel> assemblyLevel{ this, textId("assemblyLevel") };
        PropertyEnum<ShapeRepresentation> preferredShapeRepresentation{ this, textId("preferredShapeRepresentation") };
        PropertyBool readShapeAspect{ this, textId("readShapeAspect") };
        PropertyBool readSubShapesNames{ this, textId("readSubShapesNames") };
        PropertyEnum<Encoding> encoding{ this, textId("encoding") };

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccStepReader)

    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

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

    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
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

    struct Parameters : public PropertyGroup {
        PropertyEnum<Schema> schema{ this, textId("schema") };
        PropertyEnum<LengthUnit> lengthUnit{ this, textId("lengthUnit") };
        PropertyEnum<AssemblyMode> assemblyMode{ this, textId("assemblyMode") };
        PropertyEnum<FreeVertexMode> freeVertexMode{ this, textId("freeVertexMode") };
        PropertyBool writePCurves{ this, textId("writeParametericCurves") };
        PropertyBool writeSubShapesNames{ this, textId("writeSubShapesNames") };
        PropertyString headerAuthor{ this, textId("headerAuthor") };
        PropertyString headerOrganization{ this, textId("headerOrganization") };
        PropertyString headerOriginatingSystem{ this, textId("headerOriginatingSystem") };
        PropertyString headerDescription{ this, textId("headerDescription") };

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccStepWriter)

    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    STEPCAFControl_Writer* m_writer = nullptr;
    std::aligned_storage_t<sizeof(STEPCAFControl_Writer)> m_writerStorage;
    Parameters m_params;
    Schema m_schemaLastTransfer = Schema::AP214_IS;
};

} // namespace Mayo::IO
