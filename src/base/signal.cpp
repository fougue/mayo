/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "signal.h"

namespace Mayo {

std::unique_ptr<ISignalThreadHelper> globalHelper;

void setGlobalSignalThreadHelper(std::unique_ptr<ISignalThreadHelper> helper)
{
    globalHelper = std::move(helper);
}

ISignalThreadHelper* getGlobalSignalThreadHelper()
{
    return globalHelper.get();
}

} // namespace Mayo
