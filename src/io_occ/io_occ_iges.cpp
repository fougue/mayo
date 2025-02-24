/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_iges.h"
#include "io_occ_caf.h"
#include "../base/occ_static_variables_rollback.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/enumeration_fromenum.h"

#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>

namespace Mayo::IO {

class OccIgesReader::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccIgesReader::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->bsplineContinuity.setDescription(
                    textIdTr("Manages the continuity of BSpline curves (IGES entities 106, 112 and 126) "
                             "after translation to Open CASCADE (it requires that the curves "
                             "in a model be at least C1 continuous; no such requirement is made by IGES)."
                             "This parameter does not change the continuity of curves that are used "
                             "in the construction of IGES BRep entities. In this case, the parameter "
                             "does not influence the continuity of the resulting Open CASCADE curves "
                             "(it is ignored)."
                     )
            );

        this->surfaceCurveMode.setDescription(
                    textIdTr("Preference for the computation of curves in case of 2D/3D inconsistency "
                             "in an entity which has both 2D and 3D representations.\n\n"
                             "Concerned entity types are 141 (Boundary), 142 (CurveOnSurface) "
                             "and 508 (Loop). These are entities representing a contour lying on a "
                             "surface, which is translated to a TopoDS_Wire, formed by TopoDS_Edges. "
                             "Each TopoDS_Edge must have a 3D curve and a 2D curve that reference the surface.\n\n"
                             "The processor also decides to re-compute either the 3D or the 2D curve "
                             "even if both curves are translated successfully and seem to be correct, "
                             "in case there is inconsistency between them. The processor considers that "
                             "there is inconsistency if any of the following conditions is satisfied:\n"
                             "- the number of sub-curves in the 2D curve is different from the number "
                             "of sub-curves in the 3D curve. This can be either due to different numbers "
                             "of sub-curves given in the IGES file or because of splitting of curves during "
                             "translation\n"
                             "- 3D or 2D curve is a Circular Arc (entity type 100) starting and ending "
                             "in the same point (note that this case is incorrect according to the IGES standard)"
                     )
            );

        this->readFaultyEntities.setDescription(textIdTr("Read failed entities"));

        this->bsplineContinuity.setDescriptions({
                    { BSplineContinuity::NoChange, textIdTr("Curves are taken as they are in the IGES "
                      "file. C0 entities of Open CASCADE may be produced")
                    },
                    { BSplineContinuity::BreakIntoC1Pieces, textIdTr("If an IGES BSpline, Spline or CopiousData "
                      "curve is C0 continuous, it is broken down into pieces of C1 continuous Geom_BSplineCurve")
                    },
                    { BSplineContinuity::BreakIntoC2Pieces, textIdTr("IGES Spline curves are broken down "
                      "into pieces of C2 continuity. If C2 cannot be ensured, the Spline curves will be "
                      "broken down into pieces of C1 continuity")
                    }
        });

        this->surfaceCurveMode.setDescriptions({
                    { SurfaceCurveMode::Default, textIdTr("Use the preference flag value in the entity's `Parameter Data` section") },
                    { SurfaceCurveMode::Prefer2D, textIdTr("The 2D is used to rebuild the 3D in case of their inconsistency") },
                    { SurfaceCurveMode::Force2D, textIdTr("The 2D is always used to rebuild the 3D (even if 3D is present in the file)")},
                    { SurfaceCurveMode::Prefer3D, textIdTr("The 3D is used to rebuild the 2D in case of their inconsistency") },
                    { SurfaceCurveMode::Force3D, textIdTr("The 3D is always used to rebuild the 2D (even if 2D is present in the file)") },
        });
    }

    void restoreDefaults() override {
        const OccIgesReader::Parameters params;
        this->bsplineContinuity.setValue(params.bsplineContinuity);
        this->surfaceCurveMode.setValue(params.surfaceCurveMode);
        this->readFaultyEntities.setValue(params.readFaultyEntities);
        this->readOnlyVisibleEntities.setValue(params.readOnlyVisibleEntities);
    }

    PropertyEnum<BSplineContinuity> bsplineContinuity{ this, textId("bsplineContinuity") };
    PropertyEnum<SurfaceCurveMode> surfaceCurveMode{ this, textId("surfaceCurveMode") };
    PropertyBool readFaultyEntities{ this, textId("readFaultyEntities") };
    PropertyBool readOnlyVisibleEntities{ this, textId("readOnlyVisibleEntities") };
};

OccIgesReader::OccIgesReader()
{
    MayoIO_CafGlobalScopedLock(cafLock);
    m_reader = new(&m_readerStorage) IGESCAFControl_Reader();
    IGESControl_Controller::Init();
    m_reader->SetColorMode(true);
    m_reader->SetNameMode(true);
    m_reader->SetLayerMode(true);
}

OccIgesReader::~OccIgesReader()
{
    m_reader->~IGESCAFControl_Reader();
}

bool OccIgesReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafReadFile(*m_reader, filepath, progress);
}

TDF_LabelSequence OccIgesReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafTransfer(*m_reader, doc, progress);
}

std::unique_ptr<PropertyGroup> OccIgesReader::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccIgesReader::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.bsplineContinuity = ptr->bsplineContinuity;
        m_params.surfaceCurveMode = ptr->surfaceCurveMode;
        m_params.readFaultyEntities = ptr->readFaultyEntities;
        m_params.readOnlyVisibleEntities = ptr->readOnlyVisibleEntities;
    }
}

void OccIgesReader::changeStaticVariables(OccStaticVariablesRollback* rollback) const
{
    rollback->change("read.iges.bspline.continuity", int(m_params.bsplineContinuity));
    rollback->change("read.surfacecurve.mode", int(m_params.surfaceCurveMode));
    rollback->change("read.iges.faulty.entities", int(m_params.readFaultyEntities ? 1 : 0));
    rollback->change("read.iges.onlyvisible", int(m_params.readOnlyVisibleEntities ? 1 : 0));
}

class OccIgesWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccIgesWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->planeMode.setDescription(
                    textIdTr("Indicates if planes should be saved as Bsplines or Planes (type 108). "
                             "Writing p-curves on planes is disabled"));
        this->brepMode.setDescriptions({
                    { BRepMode::Faces, textIdTr("OpenCascade TopoDS_Faces will be translated into IGES 144 "
                      "(Trimmed Surface) entities, no BRep entities will be written to the IGES file")
                    },
                    { BRepMode::BRep, textIdTr("OpenCascade TopoDS_Faces will be translated into IGES 510 "
                      "(Face) entities, the IGES file will contain BRep entities")
                    }
        });
    }

    void restoreDefaults() override {
        const OccIgesWriter::Parameters params;
        this->brepMode.setValue(params.brepMode);
        this->planeMode.setValue(params.planeMode);
        this->lengthUnit.setValue(params.lengthUnit);
    }

    PropertyEnum<BRepMode> brepMode{ this, textId("brepMode") };
    PropertyEnum<PlaneMode> planeMode{ this, textId("planeMode") };
    PropertyEnum<OccCommon::LengthUnit> lengthUnit{ this, textId("lengthUnit") };
};

OccIgesWriter::OccIgesWriter()
{
    MayoIO_CafGlobalScopedLock(cafLock);
    m_writer = new(&m_writerStorage) IGESCAFControl_Writer();
    IGESControl_Controller::Init();
    m_writer->SetColorMode(true);
    m_writer->SetNameMode(true);
    m_writer->SetLayerMode(true);
}

OccIgesWriter::~OccIgesWriter()
{
    m_writer->~IGESCAFControl_Writer();
}

bool OccIgesWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafTransfer(*m_writer, appItems, progress);
}

bool OccIgesWriter::writeFile(const FilePath& filepath, TaskProgress* /*progress*/)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    m_writer->ComputeModel();
    const bool ok = m_writer->Write(filepath.u8string().c_str());
    return ok;
}

std::unique_ptr<PropertyGroup> OccIgesWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccIgesWriter::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.brepMode = ptr->brepMode;
        m_params.planeMode = ptr->planeMode;
        m_params.lengthUnit = ptr->lengthUnit;
    }
}

void OccIgesWriter::changeStaticVariables(OccStaticVariablesRollback* rollback)
{
    rollback->change("write.iges.brep.mode", int(m_params.brepMode));
    rollback->change("write.iges.plane.mode", int(m_params.planeMode));
    rollback->change("write.iges.unit", OccCommon::toCafString(m_params.lengthUnit));
}

} // namespace Mayo::IO
