/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
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
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;

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

    struct Parameters {
        BSplineContinuity bsplineContinuity = BSplineContinuity::BreakIntoC1Pieces;
        SurfaceCurveMode surfaceCurveMode = SurfaceCurveMode::Default;
        bool readFaultyEntities = false;
        bool readOnlyVisibleEntities = false;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* group) override;

private:
    void changeStaticVariables(OccStaticVariablesRollback* rollback) const;

    class Properties;
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

    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters

    enum class BRepMode {
        Faces = 0, BRep = 1
    };

    enum class PlaneMode {
        Plane = 0, BSpline = 1
    };

    using LengthUnit = OccCommon::LengthUnit;
    struct Parameters {
        BRepMode brepMode = BRepMode::Faces;
        PlaneMode planeMode = PlaneMode::Plane;
        LengthUnit lengthUnit = LengthUnit::Millimeter;
        // TODO Support "write.iges.offset.mode"
        // Summary: Writing offset-based surfaces of revolution to IGES
        // New parameter "write.iges.offset.mode" added in class
        // GeomToIGES_GeomCurve allows writing offset curves in form of b-splines.
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* group) override;

private:
    void changeStaticVariables(OccStaticVariablesRollback* rollback);

    class Properties;
    IGESCAFControl_Writer* m_writer = nullptr;
    std::aligned_storage_t<sizeof(IGESCAFControl_Writer)> m_writerStorage;
    Parameters m_params;
};

} // namespace Mayo::IO
