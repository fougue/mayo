/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>

#include <vector>

namespace Mayo {

// Provides syntax highlighting for JavaScript code
class JavaScriptSyntaxHighlighter : public QSyntaxHighlighter {
public:
    JavaScriptSyntaxHighlighter(QTextDocument* parent = nullptr);

protected:
    enum BlockState {
        MultiLineComment = 1, TemplateLiteral = 2
    };

    void highlightBlock(const QString& text) override;

private:
    void highlightWithinDelimiters(
        const QString& text,
        const char* cstrStart,
        const char* cstrEnd,
        BlockState state,
        const QTextCharFormat& charFormat
    );

    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    std::vector<HighlightingRule> m_highlightingRules;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_templateFormat;
};

} // namespace Mayo
