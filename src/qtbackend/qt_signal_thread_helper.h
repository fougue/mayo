/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/signal.h"

namespace Mayo {

// Provides handling of signal/slot thread mismatch with the help of Qt
// There will be a single QObject created per thread, so it can be used to enqueue slot functions
class QtSignalThreadHelper : public ISignalThreadHelper {
public:
    std::any getCurrentThreadContext() override;
    void execInThread(const std::any& context, const std::function<void()>& fn) override;
};

} // namespace Mayo
