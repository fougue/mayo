/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "qt_app_translator.h"

#include "../base/i18n_translation_cache.h"

#include <QtCore/QCoreApplication>

namespace Mayo {

// Function called by the i18n system, see TextId::addTranslatorFunction()
std::string_view qtAppTranslate(const TextId& text, int n)
{
    static I18nTranslationCache trCache;

    if (std::string_view tr = trCache.find(text, n); !tr.empty())
        return tr;

    thread_local std::string buffer;
    buffer.clear();
    buffer.append(text.trContext);
    buffer.push_back('\0');
    buffer.append(text.key);
    buffer.push_back('\0');
    const QString qTr = QCoreApplication::translate(
        buffer.data(), buffer.data() + text.trContext.size() + 1, nullptr, n
    );
    return trCache.insert(text, n, qTr.toStdString());
}

} // namespace Mayo
