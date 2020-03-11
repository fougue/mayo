/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// Module : GUI

#include <unordered_map>
#include <QtCore/QObject>
class QWindow;
class QWinTaskbarButton;

namespace Mayo {

class WinTaskbarGlobalProgress : public QObject {
    Q_OBJECT
public:
    WinTaskbarGlobalProgress(QObject* parent = nullptr);

    void setWindow(QWindow* window);

private:
    void onTaskProgress(quint64 taskId, int percent);
    void onTaskEnded(quint64 taskId);
    void updateTaskbar();

    std::unordered_map<quint64, int> m_mapTaskIdProgress;
    QWinTaskbarButton* m_taskbarBtn = nullptr;
    int m_globalPct = 0;
};

} // namespace Mayo
