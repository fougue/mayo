/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qt_app_translator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QReadWriteLock>

#include <unordered_map>

namespace Mayo {

// Function called by the Application i18n system, see Application::addTranslator()
std::string_view qtAppTranslate(const TextId& text, int n)
{
    const QString qstr = QCoreApplication::translate(text.trContext.data(), text.key.data(), nullptr, n);
    auto qstrHash = qHash(qstr);
    static std::unordered_map<decltype(qstrHash), std::string> mapStr;
    static QReadWriteLock mapStrLock;
    {
        QReadLocker locker(&mapStrLock);
        auto it = mapStr.find(qstrHash);
        if (it != mapStr.cend())
            return it->second;
    }

    QWriteLocker locker(&mapStrLock);
    auto [it, ok] = mapStr.insert({ qstrHash, qstr.toStdString() });
    return ok ? it->second : std::string_view{};
}

} // namespace Mayo
