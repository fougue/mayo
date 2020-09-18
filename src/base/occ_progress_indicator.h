/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Message_ProgressIndicator.hxx>

namespace Mayo {

class TaskProgress;

class OccProgressIndicator : public Message_ProgressIndicator {
public:
    OccProgressIndicator(TaskProgress* progress);

    bool Show(const bool force) override;
    bool UserBreak() override;

private:
    TaskProgress* m_progress = nullptr;
};

} // namespace Mayo
