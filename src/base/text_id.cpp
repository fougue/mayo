/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "text_id.h"

#include <vector>

namespace Mayo {

namespace {

std::vector<TextId::TranslatorFunctionPtr>& getTranslatorFunctions()
{
    static std::vector<TextId::TranslatorFunctionPtr> vec;
    return vec;
}

} // namespace

std::string_view TextId::tr(int n) const
{
    return TextId::translate(*this, n);
}

bool TextId::isEmpty() const
{
    return this->key.empty();
}

void TextId::addTranslatorFunction(TranslatorFunctionPtr fn)
{
    if (fn)
        getTranslatorFunctions().push_back(fn);
}

std::string_view TextId::translate(const TextId& textId, int n)
{
    for (auto it = getTranslatorFunctions().rbegin(); it != getTranslatorFunctions().rend(); ++it) {
        TranslatorFunctionPtr fn = *it;
        std::string_view msg = fn(textId, n);
        if (!msg.empty())
            return msg;
    }

    return textId.key;
}


} // namespace Mayo
