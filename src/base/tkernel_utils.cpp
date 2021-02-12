/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "tkernel_utils.h"

#include <Message_ProgressIndicator.hxx>

namespace Mayo {

TKernelUtils::ReturnType_StartProgressIndicator
TKernelUtils::start(const opencascade::handle<Message_ProgressIndicator>& progress)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    return Message_ProgressIndicator::Start(progress);
#else
    return progress;
#endif
}

} // namespace Mayo
