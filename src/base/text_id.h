/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <string_view>

// NOTE Use this command-line for generating/updating .ts files:
// $> lupdate -tr-function-alias QT_TRANSLATE_NOOP+=MAYO_TEXT_ID,Q_DECLARE_TR_FUNCTIONS+=MAYO_DECLARE_TEXT_ID_FUNCTIONS,tr+=textId file.pro
#define MAYO_DECLARE_TEXT_ID_FUNCTIONS(context) \
public: \
    static inline std::string_view textIdContext() { return #context; } \
    static inline Mayo::TextId textId(std::string_view sourceText) { \
        return Mayo::TextId{ #context, sourceText }; \
    } \
    static inline std::string_view textIdTr(std::string_view sourceText) { \
        return textId(sourceText).tr(); \
    } \
private:

namespace Mayo {

// Provides translatable text(message) identifier
struct TextId {
    // Context used to find the text(key) to translate, typically a class name
    std::string_view trContext;

    // Source text
    std::string_view key;

    // Returns the translation text of 'key' by querying translator functions registered with
    // addTranslatorFunction()
    // Parameter 'n' is used to support plural forms
    std::string_view tr(int n = -1) const;

    // Whether source text(key) is empty or not
    bool isEmpty() const;

    // Provides internationalization support for text output
    //     1st arg: message to be translated(TextId = context+key)
    //     2nd arg: when != -1 used to choose an appropriate form for the translation(e.g. "%n file found" vs. "%n files found")
    //     returns: translated message
    using TranslatorFunction = std::string_view(const TextId&, int);
    using TranslatorFunctionPtr = TranslatorFunction*;
    static void addTranslatorFunction(TranslatorFunctionPtr fn);
    static std::string_view translate(const TextId& textId, int n = -1);
};

} // namespace Mayo
