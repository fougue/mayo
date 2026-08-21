/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/io_reader.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"

#include <RWMesh_CoordinateSystem.hxx>
class RWMesh_CafReader;

namespace Mayo::IO {

// Base class around OpenCascade RWMesh_CafReader
class OccBaseMeshReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;

    using LengthUnit = OccCommon::LengthUnit;
    static double lengthUnitFactor(LengthUnit lenUnit);
    static LengthUnit lengthUnit(double factor);

    struct BaseParameters {
        std::string rootPrefix;
        RWMesh_CoordinateSystem systemCoordinatesConverter{RWMesh_CoordinateSystem_Undefined};
        LengthUnit systemLengthUnit{LengthUnit::Undefined};
    };
#if 0
    struct BaseParameters : public PropertyGroup {
        PropertyString rootPrefix{ this, textId("rootPrefix") };
        PropertyEnum<RWMesh_CoordinateSystem> systemCoordinatesConverter{ this, textId("systemCoordinatesConverter") };
        PropertyEnum<LengthUnit> systemLengthUnit{ this, textId("systemLengthUnit") };

        BaseParameters();
        void restoreDefaults() override;

        MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccBaseMeshReaderParameters)
    };
#endif

    BaseParameters& parameters() { return m_params; }
    const BaseParameters& constParameters() const { return m_params; }

protected:
    OccBaseMeshReader(RWMesh_CafReader& reader, BaseParameters& params);
    virtual void applyParameters();

private:
    FilePath m_filepath;
    RWMesh_CafReader& m_reader;
    BaseParameters& m_params;
};

} // namespace Mayo::IO
