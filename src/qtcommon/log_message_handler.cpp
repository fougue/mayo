/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "log_message_handler.h"
#include "qstring_conv.h"

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

std::ostream& LogMessageHandler::outputStream(QtMsgType type)
{
    if (!m_outputFilePath.empty() && m_outputFile.is_open())
        return m_outputFile;

    if (type == QtDebugMsg || type == QtInfoMsg)
        return std::cout;

    return std::cerr;
}

void LogMessageHandler::qtHandler(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg)
{
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
