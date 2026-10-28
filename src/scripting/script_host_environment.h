/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"

#include <TDF_Label.hxx>
#include <functional>

namespace Mayo {

class PropertyValueConversion;
class TaskProgress;
namespace IO { class System; }
namespace IO { class ParametersProvider; }

// Objects and callbacks provided by the Mayo host to its JavaScript API
struct ScriptHostEnvironment {
    ApplicationPtr application;
    const IO::System* ioSystem = nullptr;
    const IO::ParametersProvider* ioParametersProvider = nullptr; // TODO Rename to ioDefaultParametersProvider ?
    std::function<void(TDF_Label, TaskProgress*)> ioEntityImportPostProcess;
    const PropertyValueConversion* propertyValueConverter = nullptr;
};

} // namespace Mayo
