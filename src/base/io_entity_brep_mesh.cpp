/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_entity_brep_mesh.h"
#include "global.h"

#include <BRepMesh_IncrementalMesh.hxx>

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include "occ_progress_indicator.h"
#endif

namespace Mayo {
namespace IO {

void EntityBRepMesh::setParametersProvider(const ParametersProvider& paramsProvider)
{
    m_paramsProvider = paramsProvider;
}

bool EntityBRepMesh::requiresPostProcess(const Format& format) const
{
    return formatProvidesBRep(format);
}

void EntityBRepMesh::perform(const TDF_Label& labelEntity, TaskProgress* progress)
{
    if (!XCaf::isShape(labelEntity))
        return;

    TopoDS_Shape shapeEntity = XCaf::shape(labelEntity);
    OccBRepMeshParameters params;
    if (m_paramsProvider)
        params = m_paramsProvider(shapeEntity);

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    BRepMesh_IncrementalMesh mesher(shapeEntity, params, TKernelUtils::start(indicator));
#else
    BRepMesh_IncrementalMesh mesher(shapeEntity, params);
    MAYO_UNUSED(progress);
#endif
    MAYO_UNUSED(mesher);
}

} // namespace IO
} // namespace Mayo
