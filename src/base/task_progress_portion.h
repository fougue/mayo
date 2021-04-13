/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task_common.h"
#include <QtCore/QString>

namespace Mayo {

class TaskProgress;

// Provides progress indication of operation or sub-operation, depending of the hierarchical context
// Progress portions should always be stack-allocated :
//     {code
//         TaskProgress* progress = ...;
//         {
//             TaskProgressPortion portion(&progress->parentPortion(), 40, "Read operation");
//             readFile(filepath, &portion);
//         }
//         {
//             TaskProgressPortion portion(&progress->parentPortion(), 60, "Transfer operation");
//             transferObjectsFile(objects, &portion);
//         }
//     }code
//
class TaskProgressPortion {
public:
    TaskProgressPortion() = default;
    TaskProgressPortion(TaskProgressPortion* parentPortion, int portionSize, const QString& step = {});
    ~TaskProgressPortion();

    int value() const { return m_value; }
    void setValue(int pct);

    void setStep(const QString& title);

    const TaskProgress* progress() const { return m_progress; }
    TaskProgress* progress() { return m_progress; }

    const TaskProgressPortion* parent() const { return m_parentPortion; }
    TaskProgressPortion* parent() { return m_parentPortion; }

    bool isRoot() const { return m_parentPortion == nullptr; }
    bool isAbortRequested() const;

    // Disable copy
    TaskProgressPortion(const TaskProgressPortion&) = delete;
    TaskProgressPortion(TaskProgressPortion&&) = delete;
    TaskProgressPortion& operator=(const TaskProgressPortion&) = delete;
    TaskProgressPortion& operator=(TaskProgressPortion&&) = delete;

private:
    TaskProgressPortion(TaskProgress& progress);

    friend class TaskProgress;

    TaskProgress* m_progress = nullptr;
    TaskProgressPortion* m_parentPortion = nullptr;
    int m_portionSize = 0;
    int m_value = 0;
};

} // namespace Mayo
