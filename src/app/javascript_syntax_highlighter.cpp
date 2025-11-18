/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "javascript_syntax_highlighter.h"

namespace Mayo {

JavaScriptSyntaxHighlighter::JavaScriptSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    m_commentFormat.setForeground(Qt::green);
    //m_commentFormat.setFontItalic(true);
    m_templateFormat.setForeground(QColor(Qt::magenta).lighter());

    QTextCharFormat stringFormat;
    stringFormat.setForeground(Qt::green);

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::cyan);
    keywordFormat.setFontWeight(QFont::Bold);

    const char* allKeywords[] = {
        "in", "of", "do", "void", "with", "delete", "from", "as", "var", "let", "const",
        "function", "return", "if", "else", "for", "while", "break", "continue", "switch",
        "case", "default", "true", "false", "null", "undefined", "new", "this", "class",
        "extends", "super", "try", "catch", "finally", "throw", "typeof", "instanceof",
        "import", "export", "await"
    };
    const QString wordBoundary{"\\b"};
    for (const char* keyword : allKeywords) {
        const QString strRegExp = wordBoundary + keyword + wordBoundary;
        m_highlightingRules.push_back({ QRegularExpression{strRegExp}, keywordFormat });
    }

    // Rule for comments on a single line(// ...)
    m_highlightingRules.push_back({ QRegularExpression{"//[^\n]*"}, m_commentFormat });

    // Rule for strings within double quotes("...")
    m_highlightingRules.push_back({ QRegularExpression{"\".*?\""}, stringFormat });

    // Rule for strings within single quotes('...')
    m_highlightingRules.push_back({ QRegularExpression{"'.*?'"}, stringFormat });
}

void JavaScriptSyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply simple rules
    for (const HighlightingRule& rule : qAsConst(m_highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multilines comments
    highlightWithinDelimiters(text, "/*", "*/", BlockState::MultiLineComment, m_commentFormat);

    // Handle template literals
    highlightWithinDelimiters(text, "`", "`", BlockState::TemplateLiteral, m_templateFormat);
}

void JavaScriptSyntaxHighlighter::highlightWithinDelimiters(const QString &text, const char *cstrStart, const char *cstrEnd, BlockState state, const QTextCharFormat &charFormat)
{
    QLatin1String strStart(cstrStart);
    QLatin1String strEnd(cstrEnd);

    int startIndex = 0;
    if (previousBlockState() != state)
        startIndex = text.indexOf(strStart);

    while (startIndex >= 0) {
        const int endIndex = text.indexOf(strEnd, startIndex + 1);
        int stringLength;
        if (endIndex == -1) {
            setCurrentBlockState(state);
            stringLength = text.length() - startIndex;
        }
        else {
            stringLength = endIndex - startIndex + strEnd.size();
        }

        setFormat(startIndex, stringLength, charFormat);
        startIndex = text.indexOf(strStart, startIndex + stringLength);
    }
}

} // namespace Mayo
