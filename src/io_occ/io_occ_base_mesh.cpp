/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_base_mesh.h"

#include "../base/document.h"
#include "../base/scope_import.h"
#include "../base/occ_progress_indicator.h"
#include "../base/task_progress.h"
#include "../base/string_utils.h"
#include "../base/tkernel_utils.h"

#include <RWMesh_CafReader.hxx>

namespace Mayo {
namespace IO {

OccBaseMeshReaderProperties::OccBaseMeshReaderProperties(PropertyGroup* parentGroup)
    : PropertyGroup(parentGroup),
      rootPrefix(this, textId("rootPrefix")),
      systemCoordinatesConverter(this, textId("systemCoordinatesConverter")),
      systemLengthUnit(this, textId("systemLengthUnit"))
{
    this->rootPrefix.setDescription(tr("Prefix for generating root labels name"));
    this->systemLengthUnit.setDescription(tr("System length units to convert into while reading files"));
}

void OccBaseMeshReaderProperties::restoreDefaults()
{
    const OccBaseMeshReader::Parameters defaults;
    this->rootPrefix.setValue(defaults.rootPrefix);
    this->systemCoordinatesConverter.setValue(defaults.systemCoordinatesConverter);
    this->systemLengthUnit.setValue(defaults.systemLengthUnit);
}

double OccBaseMeshReaderProperties::lengthUnitFactor(LengthUnit lenUnit)
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

OccBaseMeshReaderProperties::LengthUnit OccBaseMeshReaderProperties::lengthUnit(double factor)
{
    if (factor < 0)
        return LengthUnit::Undefined;

    for (const LengthUnit lenUnit : MetaEnum::values<OccCommon::LengthUnit>()) {
        const double lenUnitFactor = OccBaseMeshReaderProperties::lengthUnitFactor(lenUnit);
        if (factor == lenUnitFactor)
            return lenUnit;
    }

    return LengthUnit::Undefined;
}

bool OccBaseMeshReader::readFile(const QString& filepath, TaskProgress* progress)
{
    m_filepath = filepath;
    progress->setValue(100);
    return true;
}

bool OccBaseMeshReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    this->applyParameters();
    m_reader.SetDocument(doc);
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    XCafScopeImport import(doc);
    const bool okPerform = m_reader.Perform(
                StringUtils::toUtf8<TCollection_AsciiString>(m_filepath),
                TKernelUtils::start(indicator));
    import.setConfirmation(okPerform && !TaskProgress::isAbortRequested(progress));
    return okPerform;
}

void OccBaseMeshReader::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const OccBaseMeshReaderProperties*>(params);
    if (ptr) {
        this->parameters().systemCoordinatesConverter = ptr->systemCoordinatesConverter;
        this->parameters().systemLengthUnit = ptr->systemLengthUnit;
        this->parameters().rootPrefix = ptr->rootPrefix;
    }
}

OccBaseMeshReader::OccBaseMeshReader(RWMesh_CafReader& reader)
    : m_reader(reader)
{
}

void OccBaseMeshReader::applyParameters()
{
    m_reader.SetRootPrefix(StringUtils::toUtf8<TCollection_AsciiString>(this->constParameters().rootPrefix));
    m_reader.SetSystemLengthUnit(OccBaseMeshReaderProperties::lengthUnitFactor(this->constParameters().systemLengthUnit));
    m_reader.SetSystemCoordinateSystem(this->constParameters().systemCoordinatesConverter);
}

} // namespace IO
} // namespace Mayo
