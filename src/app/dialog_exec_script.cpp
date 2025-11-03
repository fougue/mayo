/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_exec_script.h"

#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qtcore_utils.h"
#include "../qtcommon/qstring_conv.h"
#include "ui_dialog_exec_script.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QFontDatabase>
#include <QtGui/QPainter>
#include <QtGui/QSyntaxHighlighter>
#include <QtWidgets/QButtonGroup>

#include <algorithm>
#include <vector>

namespace Mayo {

namespace {

class LineNumberArea : public QWidget {
public:
    LineNumberArea(QPlainTextEdit* editor)
        : QWidget(editor),
          m_editor(editor)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        QObject::connect(
            editor, &QPlainTextEdit::updateRequest,
            this, &LineNumberArea::onEditorUpdateRequest
        );
        QObject::connect(
            editor, &QPlainTextEdit::blockCountChanged,
            this, &QWidget::updateGeometry
        );
    }

    QSize sizeHint() const override
    {
        int digits = 1;
        int max = std::max(1, m_editor->blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }

        const int space = 5 + m_editor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
        return QSize(space, 0);
    }

protected:
    void onEditorUpdateRequest(const QRect& rect, int dy)
    {
        if (dy)
            scroll(0, dy);
        else
            update(0, rect.y(), width(), rect.height());
    }

    void paintEvent(QPaintEvent* event) override
    {
        const QColor windowColor = palette().color(QPalette::Window);
        const QColor textColor = palette().color(QPalette::Text).darker();

        QPainter painter(this);
        painter.fillRect(event->rect(), windowColor);

        QTextBlock block = m_editor->cursorForPosition({}).block();
        const int viewportHeight = m_editor->viewport()->height();
        const int fontHeight = m_editor->fontMetrics().height();
        const int frameOffset = m_editor->viewport()->geometry().top();

        while (block.isValid()) {
            const QRect rect = m_editor->cursorRect(QTextCursor{block});
            const int y = rect.top() + frameOffset;
            if (y > viewportHeight)
                break;

            if ((y + fontHeight) >= 0) {
                const QString strNumber = QString::number(block.blockNumber() + 1);
                painter.setPen(textColor);
                painter.drawText(0, y, width() - 2, fontHeight, Qt::AlignRight, strNumber);
            }

            block = block.next();
        }
    }

private:
    QPlainTextEdit* m_editor = nullptr;
};


class JavaScriptHighlighter : public QSyntaxHighlighter
{
public:
    JavaScriptHighlighter(QTextDocument* parent = nullptr)
        : QSyntaxHighlighter(parent),
        m_commentStartRegExp("/\\*"),
        m_commentEndRegExp("\\*/")
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

protected:
    enum BlockState {
        MultiLineComment = 1, TemplateLiteral = 2
    };

    void highlightBlock(const QString& text) override
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

private:
    void highlightWithinDelimiters(
            const QString& text,
            const char* cstrStart,
            const char* cstrEnd,
            BlockState state,
            const QTextCharFormat& charFormat
        )
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

    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    std::vector<HighlightingRule> m_highlightingRules;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_templateFormat;
    // Regexps for multilines comments
    QRegularExpression m_commentStartRegExp;
    QRegularExpression m_commentEndRegExp;
};

} // namespace

DialogExecScript::DialogExecScript(ScriptEngine* engine, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogExecScript),
      m_scriptEngine(engine)
{
    m_ui->setupUi(this);

    {
        auto btnGroup = new QButtonGroup(this);
        btnGroup->addButton(m_ui->btn_OutputList, 0);
        btnGroup->addButton(m_ui->btn_OutputText, 1);
        btnGroup->addButton(m_ui->btn_Script, 2);
        btnGroup->setExclusive(true);
        QObject::connect(
            btnGroup, &QButtonGroup::idClicked,
            m_ui->stack_Panes, &QStackedWidget::setCurrentIndex
        );
    }

    m_ui->stack_Panes->setCurrentWidget(m_ui->page_OutputList);
    m_ui->btn_OutputList->setChecked(true);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_ui->editText_OutputText->setFont(fixedFont);
    m_ui->editText_Script->setFont(fixedFont);
    m_ui->editText_Script->setWordWrapMode(QTextOption::NoWrap);
    m_ui->editText_Script->setLineWrapMode(QPlainTextEdit::NoWrap);
    new JavaScriptHighlighter(m_ui->editText_Script->document());
    m_ui->layout_PageScript->insertWidget(0, new LineNumberArea(m_ui->editText_Script));

    {
        QFile file(filepathTo<QString>(engine->scriptFilePath()));
        if (file.open(QIODevice::ReadOnly))
            m_ui->editText_Script->setPlainText(file.readAll());
    }

    m_sigConns
        << m_scriptEngine->signalMessage.connectSlot(&DialogExecScript::addOutputMessage, this)
        << m_scriptEngine->signalEvaluateStarted.connectSlot(&DialogExecScript::onScriptEvaluateStarted, this)
        << m_scriptEngine->signalEvaluateEnded.connectSlot(&DialogExecScript::onScriptEvaluateEnded, this)
        ;
    QObject::connect(
        m_ui->btn_RestartStop, &QAbstractButton::clicked,
        this, [=]{ m_scriptEngine->runOrStopEvaluate(); }
    );
    QObject::connect(
        m_ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked,
        this, &DialogExecScript::tryCloseDialog
    );
}

DialogExecScript::~DialogExecScript()
{
    delete m_ui;
}

void DialogExecScript::onScriptEvaluateStarted()
{
    const QString strScriptFilePath = filepathTo<QString>(m_scriptEngine->scriptFilePath());
    m_ui->label_Status->setText(tr("Executing '%1'...").arg(strScriptFilePath));
    m_ui->btn_RestartStop->setText(tr("Stop"));
    m_ui->progressBar_Execution->setRange(0, 0);
    m_ui->progressBar_Execution->setValue(-1);
    m_ui->treeWidget_OutputList->clear();
    m_ui->editText_OutputText->clear();
}

void DialogExecScript::onScriptEvaluateEnded(
        ScriptEngine::Result /*evalResult*/, ScriptEngine::EndReason endReason
    )
{
    const QString strScriptFilePath = filepathTo<QString>(m_scriptEngine->scriptFilePath());
    const bool wasEvaluateStopped = endReason == ScriptEngine::EndReason::Stopped;
    if (!wasEvaluateStopped)
        m_ui->label_Status->setText(tr("Finished '%1'").arg(strScriptFilePath));
    else
        m_ui->label_Status->setText(tr("Stopped '%1'").arg(strScriptFilePath));

    m_ui->btn_RestartStop->setText(tr("Restart"));
    m_ui->progressBar_Execution->setRange(0, 100);
    m_ui->progressBar_Execution->setValue(!wasEvaluateStopped ? 100 : 0);
    for (int col = 0; col < m_ui->treeWidget_OutputList->columnCount(); ++col)
        m_ui->treeWidget_OutputList->resizeColumnToContents(col);
}

void DialogExecScript::addOutputMessage(const ScriptEngine::Message& msg)
{
    auto fnStrMsgType = [](MessageType type) -> QString {
        switch (type) {
        case MessageType::Trace: return tr("debug");
        case MessageType::Info: return tr("info");
        case MessageType::Warning: return tr("warning");
        case MessageType::Error: return tr("critical");
        default: return tr("?");
        }
    };

    auto item = new QTreeWidgetItem;
    item->setText(0, fnStrMsgType(msg.type));
    item->setFont(0, QtGuiUtils::FontChange(item->font(0)).scalePointSizeF(0.8).capitalization(QFont::AllUppercase));
    if (msg.type == MessageType::Error) {
        item->setBackground(0, QColor(Qt::red));
        item->setFont(0, QtGuiUtils::FontChange(item->font(0)).bold(true));
    }

    const QString strText = to_QString(msg.text);
    const QString strContextFile = to_QString(msg.contextFile);
    item->setText(1, strText);
    item->setText(2, QFileInfo{strContextFile}.fileName());

    // Set tooltip for the context file column
    const QUrl contextUrl{strContextFile};
    if (contextUrl.isValid() && contextUrl.isLocalFile()) {
        const QString strCanonicalContextFile = QFileInfo{contextUrl.toLocalFile()}.canonicalFilePath();
        item->setToolTip(2, QDir::toNativeSeparators(strCanonicalContextFile));
    }
    else {
        item->setToolTip(2, strContextFile);
    }

    item->setText(3, QString::number(msg.contextLine));
    m_ui->treeWidget_OutputList->addTopLevelItem(item);
    m_ui->editText_OutputText->appendPlainText(strText);
}

void DialogExecScript::tryCloseDialog()
{
    if (m_scriptEngine->isEvaluateRunning()) {
        auto msgBox = QtWidgetsUtils::asyncMsgBoxWarning(
            this,
            tr("Warning"),
            tr("Script execution isn't finished\n\nInterrupt and exit?"),
            QMessageBox::Yes | QMessageBox::No
        );
        QObject::connect(msgBox, &QMessageBox::buttonClicked, this, [=](QAbstractButton* btn) {
            if (btn == msgBox->button(QMessageBox::Yes)) {
                m_scriptEngine->stopEvaluate();
                m_scriptEngine->waitForEvaluateEnd(10000/*ms*/);
                QtCoreUtils::runJobOnMainThread([=]{ this->tryCloseDialog(); });
            }
        });
    }
    else {
        this->reject();
    }
}

} // namespace Mayo
