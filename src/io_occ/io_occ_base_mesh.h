/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/io_reader.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"

#include <RWMesh_CoordinateSystem.hxx>
class RWMesh_CafReader;

#include <QtCore/QCoreApplication>

namespace Mayo {
namespace IO {

// Base class around OpenCascade RWMesh_CafReader
class OccBaseMeshReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

    void applyProperties(const PropertyGroup* params) override;

    using LengthUnit = OccCommon::LengthUnit;
    struct Parameters {
        QString rootPrefix;
        LengthUnit systemLengthUnit = LengthUnit::Undefined;
        RWMesh_CoordinateSystem systemCoordinatesConverter = RWMesh_CoordinateSystem_Undefined;
    };
    virtual Parameters& parameters() = 0;
    virtual const Parameters& constParameters() const = 0;

protected:
    OccBaseMeshReader(RWMesh_CafReader& reader);
    virtual void applyParameters();

private:
    QString m_filepath;
    RWMesh_CafReader& m_reader;
};

// Common properties for OccBaseMeshReader
class OccBaseMeshReaderProperties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccBaseMeshReaderProperties)
    Q_DECLARE_TR_FUNCTIONS(Mayo::IO::OccBaseMeshReaderProperties)
public:
    OccBaseMeshReaderProperties(PropertyGroup* parentGroup);

    void restoreDefaults() override;

    using LengthUnit = OccBaseMeshReader::LengthUnit;
    static double lengthUnitFactor(LengthUnit lenUnit);
    static LengthUnit lengthUnit(double factor);

    PropertyQString rootPrefix;
    PropertyEnum<RWMesh_CoordinateSystem> systemCoordinatesConverter;
    PropertyEnum<LengthUnit> systemLengthUnit;
};

} // namespace IO
} // namespace Mayo
