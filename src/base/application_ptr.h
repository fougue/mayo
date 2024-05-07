/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_handle.h"
#include <XCAFApp_Application.hxx>

namespace Mayo {

class Application;
DEFINE_STANDARD_HANDLE(Application, XCAFApp_Application)
using ApplicationPtr = OccHandle<Application>;

} // namespace Mayo
