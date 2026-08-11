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

#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>

namespace Mayo::IO {

class OccStaticVariablesRollback;

// Opencascade-based reader for IGES file format
class OccIgesReader : public Reader {
public:
    OccIgesReader();
    OccIgesReader(const OccIgesReader&) = delete; // Not copyable
    OccIgesReader& operator=(const OccIgesReader&) = delete; // Not copyable
    ~OccIgesReader();

    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;

    // Parameters

    enum class BSplineContinuity {
        NoChange = 0,
        BreakIntoC1Pieces = 1,
        BreakIntoC2Pieces = 2
    };

    enum class SurfaceCurveMode {
        Default = 0,
        Prefer2D = 2,
        Force2D = -2,
        Prefer3D = 3,
        Force3D = -3
    };

    struct Parameters : public PropertyGroup {
        PropertyEnum<BSplineContinuity> bsplineContinuity{ this, textId("bsplineContinuity") };
        PropertyEnum<SurfaceCurveMode> surfaceCurveMode{ this, textId("surfaceCurveMode") };
        PropertyBool readFaultyEntities{ this, textId("readFaultyEntities") };
        PropertyBool readOnlyVisibleEntities{ this, textId("readOnlyVisibleEntities") };

        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccIgesReader)

    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    IGESCAFControl_Reader* m_reader = nullptr;
    std::aligned_storage_t<sizeof(IGESCAFControl_Reader)> m_readerStorage;
    Parameters m_params;
};

// Opencascade-based writer for IGES file format
class OccIgesWriter : public Writer {
public:
    OccIgesWriter();
    OccIgesWriter(const OccIgesWriter&) = delete; // Not copyable
    OccIgesWriter& operator=(const OccIgesWriter&) = delete; // Not copyable
    ~OccIgesWriter();

    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters

    enum class BRepMode {
        Faces = 0, BRep = 1
    };

    enum class PlaneMode {
        Plane = 0, BSpline = 1
    };

    using LengthUnit = OccCommon::LengthUnit;

    struct Parameters : public PropertyGroup {
        PropertyEnum<BRepMode> brepMode{ this, textId("brepMode") };
        PropertyEnum<PlaneMode> planeMode{ this, textId("planeMode") };
        PropertyEnum<OccCommon::LengthUnit> lengthUnit{ this, textId("lengthUnit") };

        Parameters();
        void restoreDefaults() override;

        // TODO Support "write.iges.offset.mode"
        // Summary: Writing offset-based surfaces of revolution to IGES
        // New parameter "write.iges.offset.mode" added in class
        // GeomToIGES_GeomCurve allows writing offset curves in form of b-splines.
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccIgesWriter)

    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    IGESCAFControl_Writer* m_writer = nullptr;
    std::aligned_storage_t<sizeof(IGESCAFControl_Writer)> m_writerStorage;
    Parameters m_params;
};

} // namespace Mayo::IO
