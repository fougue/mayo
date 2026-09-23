/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/occ_brep_mesh_parameters.h"
#include "../gui/gui_application.h"

namespace Mayo {

class IThreadDispatch;
class PropertyValueConversion;
class TaskProgress;
namespace IO { class System; }
namespace IO { class ParametersProvider; }

// Objects and callbacks provided by the Mayo host to the Scripting API
struct ScriptHostEnvironment {
    // Application instance exposed to scripts as `mayo.app`
    const GuiApplication* guiApp{nullptr};

    // Non-owning dispatcher for operations on the host application thread
    IThreadDispatch* threadDispatch{nullptr};

    // Non-owning converter for Mayo property values exposed through the scripting API
    const PropertyValueConversion* propertyValueConverter{nullptr};

    // Non-owning access to the host I/O system
    const IO::System* ioSystem{nullptr};

    // Non-owning source of format-specific reader parameters
    const IO::ParametersProvider* ioParametersProvider{nullptr};

    // Parameters used by the host for meshing BRep shapes
    // 📌 using MeshParams : std::variant<OccMeshParams, NetgenMeshParams, GmshMeshParams, ...>
    OccBRepMeshParameters meshingParams;
};

} // namespace Mayo
