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
#define MAYO_TEXT_ID(trContext, key) Mayo::TextId{ QByteArrayLiteral(trContext), QByteArrayLiteral(key) }

namespace Mayo {

struct TextId {
    QByteArray trContext;
    QByteArray key;

    QString tr() const;
    bool isEmpty() const;
};

} // namespace Mayo
