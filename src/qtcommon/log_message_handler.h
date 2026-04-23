/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath.h"

#include <QtCore/QMessageLogContext>
#include <QtCore/QString>

#include <fstream>

namespace Mayo {

// Provides customization of Qt message handler
class LogMessageHandler {
public:
    static LogMessageHandler& instance();

    void enableDebugLogs(bool on);
    void setOutputFilePath(const FilePath& fp);

    std::ostream& outputStream(QtMsgType type);

    // Function called for Qt message handling
    static void qtHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    LogMessageHandler() = default;

    FilePath m_outputFilePath;
    std::ofstream m_outputFile;
    bool m_enableDebugLogs = true;
};
} // namespace Mayo
