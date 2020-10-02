/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <QtCore/QString>

// NOTE Use this command-line for generating/updating .ts files:
// $> lupdate -tr-function-alias QT_TRANSLATE_NOOP+=MAYO_TEXT_ID file.pro
#define MAYO_TEXT_ID(trContext, key) { trContext, key }

namespace Mayo {

struct TextId {
    Span<const char> trContext; // TODO Change to QByteArray(from raw data)
    Span<const char> key; // TODO Change to QByteArray(from raw data)

    QString tr() const;
    bool isEmpty() const;

    explicit operator QByteArray() const {
        return QByteArray::fromRawData(this->key.data(), this->key.size());
    }

    static bool sameKey(const TextId& lhs, const TextId& rhs);
};

} // namespace Mayo
