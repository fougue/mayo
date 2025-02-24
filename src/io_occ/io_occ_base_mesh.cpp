/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_base_mesh.h"

#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/geom_utils.h"
#include "../base/messenger.h"
#include "../base/occ_progress_indicator.h"
#include "../base/task_progress.h"
#include "../base/string_conv.h"
#include "../base/tkernel_utils.h"

#include <RWMesh_CafReader.hxx>
#include <XCAFDoc_Location.hxx>

#include <cmath>
#include <fmt/format.h>

namespace Mayo::IO {

OccBaseMeshReaderProperties::OccBaseMeshReaderProperties(PropertyGroup* parentGroup)
    : PropertyGroup(parentGroup),
      rootPrefix(this, textId("rootPrefix")),
      systemCoordinatesConverter(this, textId("systemCoordinatesConverter")),
      systemLengthUnit(this, textId("systemLengthUnit"))
{
    this->rootPrefix.setDescription(textIdTr("Prefix for generating root labels name"));
    this->systemLengthUnit.setDescription(textIdTr("System length units to convert into while reading files"));
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

bool OccBaseMeshReader::readFile(const FilePath& filepath, TaskProgress* /*progress*/)
{
    m_filepath = filepath;
    return true;
}

// Helper function to fix potential OpenCacsade exception throwing with unsupported scale transformations
//     To be valid(for OpenCascade), a BRep shape transform should only be a composition of
//     translations and rotations. Scale factors <> 1 are not supported
//     This function traverses the assembly graph from `label` and fixes any instance node placement
//     regarding scale factor(it's forced to 1)
static void deepFixInstanceScaling(const TDF_Label& label, Messenger* messenger)
{
    if (XCaf::isShapeAssembly(label)) {
        for (const TDF_Label& childLabel : XCaf::shapeComponents(label))
            deepFixInstanceScaling(childLabel, messenger);
    }
    else if (XCaf::isShapeReference(label)) {
        const TopLoc_Location loc = XCaf::shapeReferenceLocation(label);
        if (GeomUtils::hasScaling(loc.Transformation())) {
            gp_Trsf trsf = loc.Transformation();
            const auto trsfBadScaleFactor = trsf.ScaleFactor();
            trsf.SetScaleFactor(1.);
            XCAFDoc_Location::Set(label, trsf);
            messenger->warning() << fmt::format(
                "Item \"{}\" is located with scaling transformation which is not supported(value={})",
                to_stdString(CafUtils::labelAttrStdName(label)),
                trsfBadScaleFactor
            );
        }

        const TDF_Label referred = XCaf::shapeReferred(label);
        deepFixInstanceScaling(referred, messenger);
    }
}

TDF_LabelSequence OccBaseMeshReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    this->applyParameters();
    m_reader.SetDocument(doc);
    const TDF_LabelSequence seqMark = doc->xcaf().topLevelFreeShapes();
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    m_reader.Perform(m_filepath.u8string().c_str(), TKernelUtils::start(indicator));
    const TDF_LabelSequence seqShapeLabel = doc->xcaf().diffTopLevelFreeShapes(seqMark);
    for (const TDF_Label& shapeLabel : seqShapeLabel)
        deepFixInstanceScaling(shapeLabel, this->messenger());

    return seqShapeLabel;
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
    m_reader.SetRootPrefix(string_conv<TCollection_AsciiString>(this->constParameters().rootPrefix));
    m_reader.SetSystemLengthUnit(OccBaseMeshReaderProperties::lengthUnitFactor(this->constParameters().systemLengthUnit));
    m_reader.SetSystemCoordinateSystem(this->constParameters().systemCoordinatesConverter);
}

} // namespace Mayo::IO
