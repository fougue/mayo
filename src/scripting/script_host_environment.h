/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"

#include <TDF_Label.hxx>
#include <functional>

namespace Mayo {

class IThreadDispatch;
class PropertyValueConversion;
class TaskProgress;
namespace IO { class System; }
namespace IO { class ParametersProvider; }

// Objects and callbacks provided by the Mayo host to the Scripting API
struct ScriptHostEnvironment {
    // Application instance exposed to scripts as `mayo.app`
    ApplicationPtr application;

    // Non-owning dispatcher for operations on the host application thread
    IThreadDispatch* threadDispatch{nullptr};

    // Non-owning access to the host's I/O system
    const IO::System* ioSystem{nullptr};

    // Non-owning source of default parameters for I/O operations
    const IO::ParametersProvider* ioParametersProvider{nullptr}; // TODO Rename to ioDefaultParametersProvider?

    // Optional callback applied to imported entities (eg meshing of BRep shapes)
    std::function<void(TDF_Label, TaskProgress*)> ioEntityImportPostProcess;

    // Non-owning converter for Mayo property values exposed through the scripting API
    const PropertyValueConversion* propertyValueConverter{nullptr};
};

} // namespace Mayo
