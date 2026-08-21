/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_iges.h"
#include "io_occ_caf.h"
#include "../base/occ_static_variables_rollback.h"
#include "../base/task_progress.h"

#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>

namespace Mayo::IO {

OccIgesReader::Parameters::Parameters()
{
    this->restoreDefaults();

    this->bsplineContinuity.setDescription(textId(
        "Manages the continuity of BSpline curves (IGES entities 106, 112 and 126) "
        "after translation to Open CASCADE (it requires that the curves "
        "in a model be at least C1 continuous; no such requirement is made by IGES)."
        "This parameter does not change the continuity of curves that are used "
        "in the construction of IGES BRep entities. In this case, the parameter "
        "does not influence the continuity of the resulting Open CASCADE curves "
        "(it is ignored)."
    ));

    this->surfaceCurveMode.setDescription(textId(
        "Preference for the computation of curves in case of 2D/3D inconsistency "
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
    ));

    this->readFaultyEntities.setDescription(textId("Read failed entities"));

    this->bsplineContinuity.setDescriptions({
        {
            BSplineContinuity::NoChange,
            textId("Curves are taken as they are in the IGES file. C0 entities of Open CASCADE may be produced")
        },
        {
            BSplineContinuity::BreakIntoC1Pieces,
            textId("If an IGES BSpline, Spline or CopiousData curve is C0 continuous, it is broken "
                   "down into pieces of C1 continuous Geom_BSplineCurve")
        },
        {
            BSplineContinuity::BreakIntoC2Pieces,
            textId("IGES Spline curves are broken down into pieces of C2 continuity. If C2 cannot "
                   "be ensured, the Spline curves will be broken down into pieces of C1 continuity")
        }
    });

    this->surfaceCurveMode.setDescriptions({
        { SurfaceCurveMode::Default, textId("Use the preference flag value in the entity's `Parameter Data` section") },
        { SurfaceCurveMode::Prefer2D, textId("The 2D is used to rebuild the 3D in case of their inconsistency") },
        { SurfaceCurveMode::Force2D, textId("The 2D is always used to rebuild the 3D (even if 3D is present in the file)")},
        { SurfaceCurveMode::Prefer3D, textId("The 3D is used to rebuild the 2D in case of their inconsistency") },
        { SurfaceCurveMode::Force3D, textId("The 3D is always used to rebuild the 2D (even if 2D is present in the file)") },
    });
}

void OccIgesReader::Parameters::restoreDefaults()
{
    this->bsplineContinuity.setValue(BSplineContinuity::BreakIntoC1Pieces);
    this->surfaceCurveMode.setValue(SurfaceCurveMode::Default);
    this->readFaultyEntities.setValue(false);
    this->readOnlyVisibleEntities.setValue(false);
}

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

NCollection_Sequence<TDF_Label> OccIgesReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);
    OccStaticVariablesRollback rollback;
    this->changeStaticVariables(&rollback);
    return Private::cafTransfer(*m_reader, doc, progress);
}

void OccIgesReader::changeStaticVariables(OccStaticVariablesRollback* rollback) const
{
    rollback->change("read.iges.bspline.continuity", static_cast<int>(m_params.bsplineContinuity.value()));
    rollback->change("read.surfacecurve.mode", static_cast<int>(m_params.surfaceCurveMode.value()));
    rollback->change("read.iges.faulty.entities", m_params.readFaultyEntities.value() ? 1 : 0);
    rollback->change("read.iges.onlyvisible", m_params.readOnlyVisibleEntities.value() ? 1 : 0);
}

OccIgesWriter::Parameters::Parameters()
{
    this->restoreDefaults();

    this->planeMode.setDescription(textId(
        "Indicates if planes should be saved as Bsplines or Planes (type 108). "
        "Writing p-curves on planes is disabled"
    ));
    this->brepMode.setDescriptions({
        {
            BRepMode::Faces,
            textId("OpenCascade TopoDS_Faces will be translated into IGES 144 (Trimmed Surface) entities, "
                   "no BRep entities will be written to the IGES file")
        },
        {
            BRepMode::BRep,
            textId("OpenCascade TopoDS_Faces will be translated into IGES 510 (Face) entities, "
                   "the IGES file will contain BRep entities")
        }
    });
}

void OccIgesWriter::Parameters::restoreDefaults()
{
    this->brepMode.setValue(BRepMode::Faces);
    this->planeMode.setValue(PlaneMode::Plane);
    this->lengthUnit.setValue(LengthUnit::Millimeter);
}

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

bool OccIgesWriter::transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress)
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

void OccIgesWriter::changeStaticVariables(OccStaticVariablesRollback* rollback) const
{
    rollback->change("write.iges.brep.mode", static_cast<int>(m_params.brepMode.value()));
    rollback->change("write.iges.plane.mode", static_cast<int>(m_params.planeMode.value()));
    rollback->change("write.iges.unit", OccCommon::toCafString(m_params.lengthUnit.value()));
}

} // namespace Mayo::IO
