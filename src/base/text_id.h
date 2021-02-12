/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <QtCore/QString>

#include <cstring>

// NOTE Use this command-line for generating/updating .ts files:
// $> lupdate -tr-function-alias QT_TRANSLATE_NOOP+=MAYO_TEXT_ID,Q_DECLARE_TR_FUNCTIONS+=MAYO_DECLARE_TEXT_ID_FUNCTIONS,tr+=textId file.pro
#define MAYO_TEXT_ID(trContext, key) Mayo::TextId{ QByteArrayLiteral(trContext), QByteArrayLiteral(key) }
#define MAYO_DECLARE_TEXT_ID_FUNCTIONS(context) \
public: \
    static inline QByteArray textIdContext() { return QByteArrayLiteral(#context); } \
    static inline Mayo::TextId textId(const char* sourceText) { \
        return Mayo::TextId{ QByteArrayLiteral(#context), QByteArray::fromRawData(sourceText, int(std::strlen(sourceText))) }; \
    } \
    static inline QString textIdTr(const char* sourceText) { \
        return textId(sourceText).tr(); \
    } \
private:

namespace Mayo {

struct TextId {
    QByteArray trContext;
    QByteArray key;

    QString tr() const;
    bool isEmpty() const;
};

} // namespace Mayo
