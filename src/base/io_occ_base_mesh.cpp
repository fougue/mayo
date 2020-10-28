/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_base_mesh.h"

#include "document.h"
#include "scope_import.h"
#include "occ_progress_indicator.h"
#include "task_progress.h"
#include <fougtools/occtools/qt_utils.h>

#include <RWMesh_CafReader.hxx>

namespace Mayo {
namespace IO {

OccBaseMeshReaderParameters::OccBaseMeshReaderParameters(PropertyGroup* parentGroup)
    : PropertyGroup(parentGroup),
      rootPrefix(this, MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "rootPrefix")),
      systemCoordinatesConverter(
          this, MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "systemCoordinatesConverter"), &enumCoordinateSystem()),
      systemLengthUnit(
          this, MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "systemLengthUnit"), &enumLengthUnit())
{
}

double OccBaseMeshReaderParameters::lengthUnitFactor(OccBaseMeshReaderParameters::LengthUnit lenUnit)
{
    switch (lenUnit) {
    case LengthUnit::Undefined: return -1;
    case LengthUnit::Micrometer: return 1e-6;
    case LengthUnit::Millimeter: return 0.001;
    case LengthUnit::Centimeter: return 0.01;
    case LengthUnit::Meter: return 1.;
    case LengthUnit::Kilometer: return 1000.;
    case LengthUnit::Inch: return 0.0254;
    case LengthUnit::Foot: return 0.3048;
    case LengthUnit::Mile: return 1609.344;
    }

    return -1;
}

OccBaseMeshReaderParameters::LengthUnit OccBaseMeshReaderParameters::lengthUnit(double factor)
{
    if (factor < 0)
        return LengthUnit::Undefined;

    for (const Enumeration::Item& enumItem : OccBaseMeshReaderParameters::enumLengthUnit().items()) {
        const auto lenUnit = static_cast<LengthUnit>(enumItem.value);
        const double lenUnitFactor = OccBaseMeshReaderParameters::lengthUnitFactor(lenUnit);
        if (factor == lenUnitFactor)
            return lenUnit;
    }

    return LengthUnit::Undefined;
}

const Enumeration& OccBaseMeshReaderParameters::enumLengthUnit()
{
    using LengthUnit = LengthUnit;
    static const Enumeration enumeration = {
        { int(LengthUnit::Undefined), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitUndefined") },
        { int(LengthUnit::Micrometer), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitMicrometer") },
        { int(LengthUnit::Millimeter), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitMillimeter") },
        { int(LengthUnit::Centimeter), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitCentimeter") },
        { int(LengthUnit::Meter), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitMeter") },
        { int(LengthUnit::Kilometer), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitKilometer") },
        { int(LengthUnit::Inch), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitInch") },
        { int(LengthUnit::Foot), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitFoot") },
        { int(LengthUnit::Mile), MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "UnitMile") }
    };
    return enumeration;
}

const Enumeration& OccBaseMeshReaderParameters::enumCoordinateSystem()
{
    static const Enumeration enumeration = {
        { RWMesh_CoordinateSystem_Undefined, MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "SystemUndefined") },
        { RWMesh_CoordinateSystem_Zup, MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "SystemPosZUp") },
        { RWMesh_CoordinateSystem_Yup, MAYO_TEXT_ID("Mayo::IO::OccBaseMeshReader", "SystemPosYUp") }
    };
    return enumeration;
}

bool OccBaseMeshReader::readFile(const QString& filepath, TaskProgress* progress)
{
    m_filepath = filepath;
    progress->setValue(100);
    return true;
}

bool OccBaseMeshReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    m_reader.SetDocument(doc);
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    XCafScopeImport import(doc);
    const bool okPerform = m_reader.Perform(occ::QtUtils::toOccUtf8String(m_filepath), indicator);
    import.setConfirmation(okPerform && !TaskProgress::isAbortRequested(progress));
    return okPerform;
}

void OccBaseMeshReader::applyProperties(const PropertyGroup *params)
{
    auto ptr = dynamic_cast<const OccBaseMeshReaderParameters*>(params);
    if (ptr) {
        this->setSystemCoordinatesConverter(
                    ptr->systemCoordinatesConverter.valueAs<RWMesh_CoordinateSystem>());
        this->setSystemLengthUnit(ptr->systemLengthUnit.valueAs<LengthUnit>());
        this->setRootPrefix(ptr->rootPrefix.value());
    }
}

QString OccBaseMeshReader::rootPrefix() const
{
    return occ::QtUtils::fromLatin1ToQString(m_reader.RootPrefix());
}

void OccBaseMeshReader::setRootPrefix(const QString& prefix)
{
    m_reader.SetRootPrefix(occ::QtUtils::toOccUtf8String(prefix));
}

OccBaseMeshReader::LengthUnit OccBaseMeshReader::systemLengthUnit() const
{
    return OccBaseMeshReaderParameters::lengthUnit(m_reader.SystemLengthUnit());
}

void OccBaseMeshReader::setSystemLengthUnit(LengthUnit len)
{
    m_reader.SetSystemLengthUnit(OccBaseMeshReaderParameters::lengthUnitFactor(len));
}

RWMesh_CoordinateSystem OccBaseMeshReader::systemCoordinatesConverter() const
{
    return m_systemCoordsConverter;
}

void OccBaseMeshReader::setSystemCoordinatesConverter(RWMesh_CoordinateSystem system)
{
    m_systemCoordsConverter = system;
    m_reader.SetSystemCoordinateSystem(system);
}

OccBaseMeshReader::OccBaseMeshReader(RWMesh_CafReader& reader)
    : m_reader(reader)
{
}

} // namespace IO
} // namespace Mayo
