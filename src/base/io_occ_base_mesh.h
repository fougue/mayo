/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_reader.h"
#include "property_builtins.h"
#include "property_enumeration.h"

#include <RWMesh_CoordinateSystem.hxx>
class RWMesh_CafReader;

namespace Mayo {
namespace IO {

// Base class around OpenCascade RWMesh_CafReader
class OccBaseMeshReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

    void applyParameters(const PropertyGroup* params) override;

    enum class LengthUnit {
        Undefined = -1,
        Micrometer,
        Millimeter,
        Centimeter,
        Meter,
        Kilometer,
        Inch,
        Foot,
        Mile
    };

    QString rootPrefix() const;
    void setRootPrefix(const QString& prefix);

    LengthUnit systemLengthUnit() const;
    void setSystemLengthUnit(LengthUnit len);

    RWMesh_CoordinateSystem systemCoordinatesConverter() const;
    void setSystemCoordinatesConverter(RWMesh_CoordinateSystem system);

protected:
    OccBaseMeshReader(RWMesh_CafReader& reader);

private:
    QString m_filepath;
    RWMesh_CoordinateSystem m_fileCoordsConverter = RWMesh_CoordinateSystem_Undefined;
    RWMesh_CoordinateSystem m_systemCoordsConverter = RWMesh_CoordinateSystem_Undefined;
    RWMesh_CafReader& m_reader;
};

// Common parameters for OccBaseMeshReader
struct OccBaseMeshReaderParameters : public PropertyGroup {
    OccBaseMeshReaderParameters(PropertyGroup* parentGroup);

    using LengthUnit = OccBaseMeshReader::LengthUnit;
    static double lengthUnitFactor(LengthUnit lenUnit);
    static LengthUnit lengthUnit(double factor);
    static const Enumeration& enumLengthUnit();

    static const Enumeration& enumCoordinateSystem();

    PropertyQString rootPrefix;
    PropertyEnumeration systemCoordinatesConverter;
    PropertyEnumeration systemLengthUnit;
};

} // namespace IO
} // namespace Mayo
