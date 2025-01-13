/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
