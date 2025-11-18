/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_exec_script.h"

#include "javascript_syntax_highlighter.h"
#include "line_edit_extra.h"
#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "theme.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qtcore_utils.h"
#include "../qtcommon/qstring_conv.h"
#include "ui_dialog_exec_script.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QFontDatabase>
#include <QtGui/QPainter>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QWidgetAction>

#include <algorithm>

namespace Mayo {

namespace {

// Provides QPlainTextEdit side widget to display line numbers
class LineNumberArea : public QWidget {
public:
    LineNumberArea(QPlainTextEdit* editor)
        : QWidget(editor),
          m_editor(editor)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        QObject::connect(
            editor, &QPlainTextEdit::updateRequest, this, &LineNumberArea::onEditorUpdateRequest
        );
        QObject::connect(
            editor, &QPlainTextEdit::blockCountChanged, this, &QWidget::updateGeometry
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
        return QSize{space, 0};
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

        const int viewportHeight = m_editor->viewport()->height();
        const int fontHeight = m_editor->fontMetrics().height();
        const int frameOffset = m_editor->viewport()->geometry().top();

        QTextBlock block = m_editor->cursorForPosition({}).block();
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

// Helper function to get `action` check state(if it's checkable)
Qt::CheckState getActionCheckState(const QAction* action)
{
    auto wAction = dynamic_cast<const QWidgetAction*>(action);
    auto btn = wAction ? dynamic_cast<const QAbstractButton*>(wAction->defaultWidget()) : nullptr;
    if (btn && btn->isCheckable())
        return btn->isChecked() ? Qt::Checked : Qt::Unchecked;

    if (action->isCheckable())
        return action->isChecked() ? Qt::Checked : Qt::Unchecked;

    return Qt::Unchecked;
}

// Helper function create QWidgetAction wrapping a checkbox
// QWidgetAction objects are used to avoid auto closing of the menu when an action is triggered
std::pair<QAbstractButton*, QAction*> createCheckAction(
        const QString& text, QObject* parent, Qt::CheckState state = Qt::Unchecked
    )
{
    auto checkBox = new QCheckBox(text);
    checkBox->setCheckState(state);
    auto action = new QWidgetAction(parent);
    action->setDefaultWidget(checkBox);
    return {checkBox, action};
}

} // namespace

// Provides Qt model for "Output List" panel
class DialogExecScript::OutputListModel : public QAbstractTableModel {
public:
    enum Column {
        Type = 0, Text, ContextFile, ContextLine
    };

    OutputListModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent),
        m_backgroundError(Qt::red)
    {
        m_fontTypeCommon = QtGuiUtils::FontChange(QFont{}).scalePointSizeF(0.8).capitalization(QFont::AllUppercase);
        m_fontTypeError = QtGuiUtils::FontChange(m_fontTypeCommon).bold(true);
    }

    void addMessage(const ScriptEngine::Message& msg)
    {
        const int rowMsg = int(m_messages.size());
        this->beginInsertRows(QModelIndex{}, rowMsg, rowMsg);

        const QString strContextFile = to_QString(msg.contextFile);

        Message qmsg;
        qmsg.type = msg.type;
        qmsg.text = to_QString(msg.text);
        qmsg.contextFile = QFileInfo{strContextFile}.fileName();
        qmsg.contextCanonicalFilePath = strContextFile;
        qmsg.contextLine = QString::number(msg.contextLine);

        const QUrl contextUrl{strContextFile};
        if (contextUrl.isValid() && contextUrl.isLocalFile()) {
            const QString strCanonicalContextFile = QFileInfo{contextUrl.toLocalFile()}.canonicalFilePath();
            qmsg.contextCanonicalFilePath = QDir::toNativeSeparators(strCanonicalContextFile);
        }

        m_messages.push_back(std::move(qmsg));

        this->endInsertRows();
    }

    void clear()
    {
        this->beginResetModel();
        m_messages.clear();
        this->endResetModel();
    }

    int rowCount(const QModelIndex& /*parent*/) const override
    {
        return int(m_messages.size());
    }

    int columnCount(const QModelIndex& /*parent*/) const override
    {
        return 4;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        const Message& msg = m_messages.at(index.row());

        switch (index.column()) {
        case Column::Type: {
            if (role == Qt::DisplayRole) {
                switch (msg.type) {
                case MessageType::Trace: return DialogExecScript::tr("debug");
                case MessageType::Info: return DialogExecScript::tr("info");
                case MessageType::Warning: return DialogExecScript::tr("warning");
                case MessageType::Error: return DialogExecScript::tr("critical");
                default: return DialogExecScript::tr("?");
                }
            }
            else if (role == Qt::FontRole) {
                if (msg.type == MessageType::Error)
                    return m_fontTypeError;
                else
                    return m_fontTypeCommon;
            }
            else if (role == Qt::BackgroundRole) {
                if (msg.type == MessageType::Error)
                    return m_backgroundError;
            }

            break;
        }
        case Column::Text: {
            if (role == Qt::DisplayRole)
                return msg.text;

            break;
        }
        case Column::ContextFile: {
            if (role == Qt::DisplayRole)
                return msg.contextFile;
            else if (role == Qt::ToolTipRole)
                return msg.contextCanonicalFilePath;

            break;
        }
        case Column::ContextLine: {
            if (role == Qt::DisplayRole)
                return msg.contextLine;

            break;
        }
        } // endswitch()

        return QVariant{};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == Column::Type)
                return DialogExecScript::tr("Type");
            else if (section == Column::Text)
                return DialogExecScript::tr("Message");
            else if (section == Column::ContextFile)
                return DialogExecScript::tr("File");
            else if (section == Column::ContextLine)
                return DialogExecScript::tr("Line");
        }

        return QAbstractTableModel::headerData(section, orientation, role);
    }

private:
    struct Message {
        MessageType type = MessageType::Trace;
        QString text;
        QString contextFile;
        QString contextCanonicalFilePath;
        QString contextLine;
    };

    std::vector<Message> m_messages;
    QFont m_fontTypeCommon;
    QFont m_fontTypeError;
    QBrush m_backgroundError;
};

DialogExecScript::DialogExecScript(ScriptEngine* engine, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogExecScript),
      m_fileSystemWatcher(new QFileSystemWatcher(this)),
      m_scriptEngine(engine)
{
    m_ui->setupUi(this);

    // Initialize Qt model for "Output List" panel
    {
        auto filterModel = new QSortFilterProxyModel(this);
        filterModel->setFilterKeyColumn(OutputListModel::Column::Text);
        filterModel->setSourceModel(new OutputListModel(this));
        m_ui->treeView_OutputList->setModel(filterModel);
        m_ui->treeView_OutputList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }

    // Create QButtonGroup for the tab bar buttons
    {
        auto btnGroup = new QButtonGroup(this);
        btnGroup->addButton(m_ui->btn_OutputList, 0);
        btnGroup->addButton(m_ui->btn_OutputText, 1);
        btnGroup->addButton(m_ui->btn_Script, 2);
        btnGroup->setExclusive(true);
        QObject::connect(
            btnGroup, &QButtonGroup::idClicked, m_ui->stack_Panes, &QStackedWidget::setCurrentIndex
        );
    }

    // Set "Filter" edit in "Output List" panel
    installFilterLineEdit(
        m_ui->edit_OutputListFilter,
        TextFilter::Option::All,
        [=](const TextFilter& filter) { this->applyOutputListFilter(filter); }
    );
    m_ui->btn_OutputListFilter->setIcon(mayoTheme()->icon(Theme::Icon::Filter));

    // Set "Filter" edit in "Output Text" panel
    installFilterLineEdit(
        m_ui->edit_OutputTextFilter,
        TextFilter::Option::All,
        [=](const TextFilter& filter) { this->applyOutputTextFilter(filter); }
    );
    {
        auto menu = new QMenu(this);
        menu->addAction(createCheckAction(tr("INFO"), this, Qt::Checked).second);
        menu->addAction(createCheckAction(tr("WARNING"), this, Qt::Checked).second);
        menu->addAction(createCheckAction(tr("CRITICAL"), this, Qt::Checked).second);
        m_ui->btn_OutputListFilter->setMenu(menu);
    }

    // Set "Output List" as starting panel
    m_ui->stack_Panes->setCurrentWidget(m_ui->page_OutputList);
    m_ui->stack_PaneToolbars->setCurrentWidget(m_ui->page_OutputListToolbar);
    m_ui->btn_OutputList->setChecked(true);

    // Initialize panels
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_ui->editText_OutputText->setFont(fixedFont);
    m_ui->editText_Script->setFont(fixedFont);
    m_ui->editText_Script->setWordWrapMode(QTextOption::NoWrap);
    m_ui->editText_Script->setLineWrapMode(QPlainTextEdit::NoWrap);
    new JavaScriptSyntaxHighlighter(m_ui->editText_Script->document());
    m_ui->layout_PageScript->insertWidget(0, new LineNumberArea(m_ui->editText_Script));

    // Load script file contents into text editor
    {
        QFile file(filepathTo<QString>(engine->scriptFilePath()));
        if (file.open(QIODevice::ReadOnly))
            m_ui->editText_Script->setPlainText(file.readAll());
    }

    // Signal/slot connections
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
    QObject::connect(
        m_ui->treeView_OutputList, &QAbstractItemView::doubleClicked,
        this, &DialogExecScript::onOutputListItemClicked
    );
    QObject::connect(
        m_ui->stack_Panes, &QStackedWidget::currentChanged,
        m_ui->stack_PaneToolbars, &QStackedWidget::setCurrentIndex
    );
    QObject::connect(
        m_fileSystemWatcher, &QFileSystemWatcher::fileChanged,
        this, &DialogExecScript::onFileChanged
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
    this->outputListModel()->clear();
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
}

void DialogExecScript::addOutputMessage(const ScriptEngine::Message& msg)
{
    this->outputListModel()->addMessage(msg);
    m_ui->editText_OutputText->appendPlainText(to_QString(msg.text));
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

void DialogExecScript::onOutputListItemClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    const QString strScriptFileName = filepathTo<QString>(m_scriptEngine->scriptFilePath().filename());
    if (index.siblingAtColumn(OutputListModel::Column::ContextFile).data() != strScriptFileName)
        return;

    const int lineNumber = index.siblingAtColumn(OutputListModel::Column::ContextLine).data().toInt();
    auto editor = m_ui->editText_Script;
    if (lineNumber < 1 || lineNumber > editor->blockCount())
        return;

    const QTextBlock block = editor->document()->findBlockByNumber(lineNumber - 1);
    if (block.isValid()) {
        editor->setTextCursor(QTextCursor{block});
        editor->centerCursor();
    }

    m_ui->btn_Script->setChecked(true);
    m_ui->stack_Panes->setCurrentWidget(m_ui->page_Script);
}

void DialogExecScript::onFileChanged(const QString& path)
{
    if (!filepathEquivalent(m_scriptEngine->scriptFilePath(), filepathFrom(path)))
        return;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
        m_ui->editText_Script->setPlainText(file.readAll());
}

void DialogExecScript::installFilterLineEdit(
        QLineEdit* lineEdit, TextFilter::Options options, ApplyTextFilter fnApplyFilter
    )
{
    auto lineEditExtra = new LineEditExtra(lineEdit);
    auto menuOptions = new QMenu(lineEdit);

    // Helper function to get current TextFilter object from the Option menu
    auto fnGetTextFilter = [=]{
        TextFilter filter;
        filter.key = lineEdit->text();
        if (getActionCheckState(menuOptions->actions().at(0)) == Qt::Checked)
            filter.options |= TextFilter::Option::UseRegExp;
        if (getActionCheckState(menuOptions->actions().at(1)) == Qt::Checked)
            filter.options |= TextFilter::Option::CaseSensitive;
        return filter;
    };
    // Helper function create QWidgetAction wrapping a checkbox
    // QWidgetAction objects are used to avoid auto closing of the menu when an action is triggered
    auto fnCreateCheckAction = [=](const QString& text, Qt::CheckState state = Qt::Unchecked) {
        auto [btn, action] = createCheckAction(text, lineEdit, state);
        QObject::connect(
            btn, &QAbstractButton::toggled, lineEdit, [=]{ fnApplyFilter(fnGetTextFilter()); }
        );
        return action;
    };

    if (options & TextFilter::Option::UseRegExp)
        menuOptions->addAction(fnCreateCheckAction(tr("Use regular expressions")));

    if (options & TextFilter::Option::CaseSensitive)
        menuOptions->addAction(fnCreateCheckAction(tr("Case sensitive")));

    lineEditExtra->setButtonMenu(LineEditExtra::Side::Left, menuOptions);
    lineEditExtra->setButtonIcon(LineEditExtra::Side::Left, mayoTheme()->icon(Theme::Icon::Magnifier));
    lineEditExtra->setButtonVisible(LineEditExtra::Side::Left, true);

    const QIcon iconClearBtn = lineEdit->style()->standardIcon(QStyle::QStyle::SP_LineEditClearButton);
    lineEditExtra->setButtonIcon(LineEditExtra::Side::Right, iconClearBtn);
    lineEditExtra->setButtonVisible(LineEditExtra::Side::Right, true);
    lineEditExtra->setButtonAutoHide(LineEditExtra::Side::Right, true);
    QObject::connect(
        lineEditExtra, &LineEditExtra::rightButtonClicked, lineEdit, &QLineEdit::clear
    );
    QObject::connect(
        lineEdit, &QLineEdit::textChanged, lineEdit, [=]{ fnApplyFilter(fnGetTextFilter()); }
    );
}

DialogExecScript::OutputListModel* DialogExecScript::outputListModel() const
{
    auto proxyModel = dynamic_cast<QSortFilterProxyModel*>(m_ui->treeView_OutputList->model());
    return dynamic_cast<OutputListModel*>(proxyModel->sourceModel());
}

void DialogExecScript::applyOutputListFilter(const TextFilter& filter)
{
    auto filterModel = dynamic_cast<QSortFilterProxyModel*>(m_ui->treeView_OutputList->model());
    const bool isCaseSensitive = filter.options & TextFilter::CaseSensitive;
    filterModel->setFilterCaseSensitivity(isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    if (filter.options & TextFilter::UseRegExp)
        filterModel->setFilterRegularExpression(filter.key);
    else
        filterModel->setFilterFixedString(filter.key);
}

void DialogExecScript::applyOutputTextFilter(const TextFilter& filter)
{
}

} // namespace Mayo
