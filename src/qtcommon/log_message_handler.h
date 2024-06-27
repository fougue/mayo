/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"

#include <QtCore/QMessageLogContext>
#include <QtCore/QString>

#include <fstream>
#include <functional>

namespace Mayo {

// Provides customization of Qt messages handling
class LogMessageHandler {
public:
    // LogMessageHandler is a singleton
    static LogMessageHandler& instance();

    // Enable debug messages(QtDebugMsg type) to be logged to output
    // If 'off' then debug messages are skipped('on' by default)
    void enableDebugLogs(bool on);

    // Log all messages to some output file
    // If 'fp' is empty then default output is used(ie std::cout/cerr objects)
    void setOutputFilePath(const FilePath& fp);

    // Return the output stream used to log messages of type 'msgType'
    std::ostream& outputStream(QtMsgType msgType);

    // Override handling of JS console messages with a callback function
    // JS console messages are identified by category "qml" and "js"
    // When a callback function is specified then such messages are "transferred" to that function(no
    // callback by default)
    using JsConsoleOutputHandler = std::function<void(QtMsgType, const QMessageLogContext&, const QString&)>;
    const JsConsoleOutputHandler& jsConsoleOutputHandler() const;
    void setJsConsoleOutputHandler(JsConsoleOutputHandler fnHandler);

    // Function called for Qt message handling
    // Pointer to this function should be passed to qInstallMessageHandler()
    static void qtHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    LogMessageHandler() = default;

    FilePath m_outputFilePath;
    std::ofstream m_outputFile;
    bool m_enableDebugLogs = true;
    JsConsoleOutputHandler m_jsConsoleOutputHandler;
};

} // namespace Mayo
