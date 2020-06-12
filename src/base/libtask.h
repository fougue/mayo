/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

//#include <fougtools/qttools/task/manager.h>

namespace qttask {

class Manager;
class Progress;
struct StdAsync;

} // namespace qttask

namespace Mayo {

using TaskManager = qttask::Manager;
using TaskProgress = qttask::Progress;
using StdAsync = qttask::StdAsync;

} // namespace Mayo
