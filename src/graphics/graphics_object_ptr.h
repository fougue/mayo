/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"
#include <AIS_InteractiveObject.hxx>

namespace Mayo {

using GraphicsObjectPtr = OccHandle<AIS_InteractiveObject>;
using GraphicsObjectSelectionMode = int;

} // namespace Mayo
