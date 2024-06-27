/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "log_message_handler.h"
#include "qstring_conv.h"

#include <cstring>
#include <iostream>

namespace Mayo {

LogMessageHandler& LogMessageHandler::instance()
{
    static LogMessageHandler object;
    return object;
}

void LogMessageHandler::enableDebugLogs(bool on)
{
    m_enableDebugLogs = on;
}

void LogMessageHandler::setOutputFilePath(const FilePath& fp)
{
    m_outputFilePath = fp;
    if (!fp.empty())
        m_outputFile.open(fp, std::ios::out | std::ios::app);
    else
        m_outputFile.close();
}

std::ostream& LogMessageHandler::outputStream(QtMsgType msgType)
{
    if (!m_outputFilePath.empty() && m_outputFile.is_open())
        return m_outputFile;

    if (msgType == QtDebugMsg || msgType == QtInfoMsg)
        return std::cout;

    return std::cerr;
}

const LogMessageHandler::JsConsoleOutputHandler& LogMessageHandler::jsConsoleOutputHandler() const
{
    return m_jsConsoleOutputHandler;
}

void LogMessageHandler::setJsConsoleOutputHandler(JsConsoleOutputHandler fnHandler)
{
    m_jsConsoleOutputHandler = std::move(fnHandler);
}

void LogMessageHandler::qtHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // Use QMessageLogContext::category and check it's "qml" or "js" to maybe transfer message to
    // another handler
    const auto& jsConsoleOutputHandler = LogMessageHandler::instance().jsConsoleOutputHandler();
    if (
        jsConsoleOutputHandler
        && (std::strcmp(context.category, "qml") == 0 || std::strcmp(context.category, "js") == 0)
        )
    {
        jsConsoleOutputHandler(type, context, msg);
        return;
    }

    const std::string localMsg = consoleToPrintable(msg);
    std::ostream& outs = LogMessageHandler::instance().outputStream(type);
    switch (type) {
    case QtDebugMsg:
        if (LogMessageHandler::instance().m_enableDebugLogs) {
            outs << "DEBUG: " << localMsg << std::endl;
        }
        break;
    case QtInfoMsg:
        outs << "INFO: " << localMsg << std::endl;
        break;
    case QtWarningMsg:
        outs << "WARNING: " << localMsg << std::endl;
        break;
    case QtCriticalMsg:
        outs << "CRITICAL: " << localMsg << std::endl;
        break;
    case QtFatalMsg:
        outs << "FATAL: " << localMsg << std::endl;
        break;
    }
}

} // namespace Mayo
