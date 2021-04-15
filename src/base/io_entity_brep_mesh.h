/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_system.h"
#include "occ_brep_mesh_parameters.h"
#include <functional>

namespace Mayo {
namespace IO {

// Provides meshing of BRep shape entities with OpenCascade's built-in mesher
class EntityBRepMesh : public IO::System::EntityPostProcess {
public:
    using ParametersProvider = std::function<OccBRepMeshParameters(const TopoDS_Shape&)>;
    void setParametersProvider(const ParametersProvider& paramsProvider);

    bool requiresPostProcess(const IO::Format& format) const override;
    void perform(const TDF_Label& labelEntity, TaskProgress* progress) override;

private:
    ParametersProvider m_paramsProvider = nullptr;
};

} // namespace IO
} // namespace Mayo
