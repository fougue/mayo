/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "occ_handle.h"
#include <XCAFApp_Application.hxx>

namespace Mayo {

class Application;
DEFINE_STANDARD_HANDLE(Application, XCAFApp_Application)
using ApplicationPtr = OccHandle<Application>;

} // namespace Mayo
