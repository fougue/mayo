/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_file.h"

#include "../base/application.h"
#include "../base/task_manager.h"
#include "../gui/gui_application.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "app_module.h"
#include "recent_files.h"
#include "theme.h"

#include <cassert>
#include <fmt/format.h>
#include <QtCore/QtDebug>
#include <QtCore/QElapsedTimer>
#include <QtCore/QMimeData>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>

namespace Mayo {

namespace {

QString fileFilter(IO::Format format)
{
    if (format == IO::Format_Unknown)
        return {};

    QString filter;
    for (std::string_view suffix : IO::formatFileSuffixes(format)) {
        if (suffix.data() != IO::formatFileSuffixes(format).front().data())
            filter += " ";

        const QString qsuffix = to_QString(suffix);
        filter += "*." + qsuffix;
#ifdef Q_OS_UNIX
        filter += " *." + qsuffix.toUpper();
#endif
    }

    //: %1 is the format identifier and %2 is the file filters string
    return Command::tr("%1 files(%2)")
            .arg(to_QString(IO::formatIdentifier(format)))
            .arg(filter);
}

IO::Format formatFromFilter(const QString& filter)
{
    for (IO::Format format : AppModule::get()->ioSystem()->readerFormats()) {
        if (filter == fileFilter(format))
            return format;
    }

    for (IO::Format format : AppModule::get()->ioSystem()->writerFormats()) {
        if (filter == fileFilter(format))
            return format;
    }

    return IO::Format_Unknown;
}

// TODO: move in Options
struct ImportExportSettings {
    FilePath openDir;
    QString selectedFilter;

    static ImportExportSettings load()
    {
        return {
            AppModule::get()->properties()->lastOpenDir.value(),
            to_QString(AppModule::get()->properties()->lastSelectedFormatFilter.value())
        };
    }

    static void save(const ImportExportSettings& sets)
    {
        AppModule::get()->properties()->lastOpenDir.setValue(sets.openDir);
        AppModule::get()->properties()->lastSelectedFormatFilter.setValue(to_stdString(sets.selectedFilter));
    }
};

struct OpenFileNames {
    std::vector<FilePath> listFilepath;
    ImportExportSettings lastIoSettings;
    IO::Format selectedFormat;

    enum GetOption {
        GetOne,
        GetMany
    };

    static OpenFileNames get(
            QWidget* parentWidget,
            OpenFileNames::GetOption option = OpenFileNames::GetMany
        )
    {
        OpenFileNames result;
        result.selectedFormat = IO::Format_Unknown;
        result.lastIoSettings = ImportExportSettings::load();
        QStringList listFormatFilter;
        for (IO::Format format : AppModule::get()->ioSystem()->readerFormats())
            listFormatFilter += fileFilter(format);

        const QString allFilesFilter = Command::tr("All files(*.*)");
        listFormatFilter.append(allFilesFilter);
        const QString dlgTitle = Command::tr("Select Part File");
        const QString dlgOpenDir = filepathTo<QString>(result.lastIoSettings.openDir);
        const QString dlgFilter = listFormatFilter.join(QLatin1String(";;"));
        QString* dlgPtrSelFilter = &result.lastIoSettings.selectedFilter;
        if (option == OpenFileNames::GetOne) {
            const QString strFilepath =
                QFileDialog::getOpenFileName(
                    parentWidget, dlgTitle, dlgOpenDir, dlgFilter, dlgPtrSelFilter
                );
            result.listFilepath.clear();
            result.listFilepath.push_back(filepathFrom(strFilepath));
        }
        else {
            const QStringList listStrFilePath =
                QFileDialog::getOpenFileNames(
                    parentWidget, dlgTitle, dlgOpenDir, dlgFilter, dlgPtrSelFilter
                );
            result.listFilepath.clear();
            for (const QString& strFilePath : listStrFilePath)
                result.listFilepath.push_back(filepathFrom(strFilePath));
        }

        if (!result.listFilepath.empty()) {
            result.lastIoSettings.openDir = result.listFilepath.front();
            result.selectedFormat =
                result.lastIoSettings.selectedFilter != allFilesFilter ?
                    formatFromFilter(result.lastIoSettings.selectedFilter) :
                    IO::Format_Unknown
                ;
            ImportExportSettings::save(result.lastIoSettings);
        }

        return result;
    }
};

QString strFilepathQuoted(const QString& filepath)
{
    for (QChar c : filepath) {
        if (c.isSpace())
            return "\"" + filepath + "\"";
    }

    return filepath;
}

} // namespace


void FileCommandTools::closeDocument(IAppContext* context, Document::Identifier docId)
{
    auto app = context->guiApp()->application();
    DocumentPtr doc = app->findDocumentByIdentifier(docId);
    app->closeDocument(doc);
}

void FileCommandTools::openDocumentsFromList(IAppContext* context, Span<const FilePath> listFilePath)
{
    assert(context != nullptr);
    auto app = context->guiApp()->application();
    auto appModule = AppModule::get();
    for (const FilePath& fp : listFilePath) {
        DocumentPtr docPtr = app->findDocumentByLocation(fp);
        if (docPtr.IsNull()) {
            docPtr = app->newDocument();
            docPtr->setName(fp.filename().u8string());
            docPtr->setFilePath(fp);
            // Use the Document identifier instead of handle within the job function(capture)
            // Using the handle increases the ref count and the task will be released on next
            // task creation, so the document won't be destroyed
            const Document::Identifier newDocId = docPtr->identifier();
            const TaskId taskId = context->taskMgr()->newTask([=](TaskProgress* progress) {
                QElapsedTimer chrono;
                chrono.start();
                const bool okImport =
                    appModule->ioSystem()->importInDocument()
                        .targetDocument(app->findDocumentByIdentifier(newDocId))
                        .withFilepath(fp)
                        .withParametersProvider(appModule)
                        .withEntityPostProcess([=](TDF_Label labelEntity, TaskProgress* progress) {
                            appModule->computeBRepMesh(labelEntity, progress);
                        })
                        .withEntityPostProcessRequiredIf(&IO::formatProvidesBRep)
                        .withEntityPostProcessInfoProgress(20, Command::textIdTr("Mesh BRep shapes"))
                        .withMessenger(appModule)
                        .withTaskProgress(progress)
                    .execute();
                if (okImport)
                    appModule->emitInfo(fmt::format(Command::textIdTr("Import time: {}ms"), chrono.elapsed()));
            });
            context->taskMgr()->setTitle(taskId, fp.stem().u8string());
            context->taskMgr()->run(taskId);
            appModule->prependRecentFile(fp);
        }
        else {
            if (listFilePath.size() == 1)
                context->setCurrentDocument(docPtr->identifier());
        }
    } // endfor()
}

void FileCommandTools::openDocument(IAppContext* context, const FilePath& filePath)
{
    FileCommandTools::openDocumentsFromList(context, Span<const FilePath>(&filePath, 1));
}

void FileCommandTools::importInDocument(
        IAppContext* context, const DocumentPtr& targetDoc, Span<const FilePath> listFilePaths
    )
{
    auto appModule = AppModule::get();
    const Document::Identifier targetDocId = targetDoc->identifier();
    const TaskId taskId = context->taskMgr()->newTask([=](TaskProgress* progress) {
        QElapsedTimer chrono;
        chrono.start();

        auto doc = appModule->application()->findDocumentByIdentifier(targetDocId);
        const bool okImport =
            appModule->ioSystem()->importInDocument()
                .targetDocument(doc)
                .withFilepaths(listFilePaths)
                .withParametersProvider(appModule)
                .withEntityPostProcess([=](TDF_Label labelEntity, TaskProgress* progress) {
                    appModule->computeBRepMesh(labelEntity, progress);
                })
                .withEntityPostProcessRequiredIf(&IO::formatProvidesBRep)
                .withEntityPostProcessInfoProgress(20, Command::textIdTr("Mesh BRep shapes"))
                .withMessenger(appModule)
                .withTaskProgress(progress)
            .execute();
        if (okImport)
            appModule->emitInfo(fmt::format(Command::textIdTr("Import time: {}ms"), chrono.elapsed()));
    });
    const QString taskTitle =
        listFilePaths.size() > 1 ?
            Command::tr("Import") :
            filepathTo<QString>(listFilePaths.front().stem())
        ;
    context->taskMgr()->setTitle(taskId, to_stdString(taskTitle));
    context->taskMgr()->run(taskId);
}

void FileCommandTools::importInDocument(
        IAppContext* context, const DocumentPtr& targetDoc, const FilePath& filePath
    )
{
    FileCommandTools::importInDocument(context, targetDoc, Span<const FilePath>(&filePath, 1));
}

CommandNewDocument::CommandNewDocument(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("New"));
    action->setToolTip(Command::tr("New Document"));
    action->setShortcut(Qt::CTRL | Qt::Key_N);
    this->setAction(action);
}

void CommandNewDocument::execute()
{
    static unsigned docSequenceId = 0;
    auto docPtr = this->app()->newDocument(Document::Format::Binary);
    docPtr->setName(to_stdString(Command::tr("Anonymous%1").arg(++docSequenceId)));
}

CommandOpenDocuments::CommandOpenDocuments(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Open"));
    action->setToolTip(Command::tr("Open Documents"));
    action->setShortcut(Qt::CTRL | Qt::Key_O);
    this->setAction(action);

    context->widgetMain()->setAcceptDrops(true);
    context->widgetMain()->installEventFilter(this);
}

void CommandOpenDocuments::execute()
{
    const auto resFileNames = OpenFileNames::get(this->widgetMain());
    if (!resFileNames.listFilepath.empty())
        FileCommandTools::openDocumentsFromList(this->context(), resFileNames.listFilepath);
}

bool CommandOpenDocuments::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this->widgetMain()) {
        if (event->type() == QEvent::DragEnter) {
            auto dragEnterEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEnterEvent->mimeData()->hasUrls())
                dragEnterEvent->acceptProposedAction();

            return true;
        }
        else if (event->type() == QEvent::Drop) {
            auto dropEvent = static_cast<QDropEvent*>(event);
            const QList<QUrl> listUrl = dropEvent->mimeData()->urls();
            std::vector<FilePath> listFilePath;
            for (const QUrl& url : listUrl) {
                if (url.isLocalFile())
                    listFilePath.push_back(filepathFrom(url.toLocalFile()));
            }

            dropEvent->acceptProposedAction();
            FileCommandTools::openDocumentsFromList(this->context(), listFilePath);
            return true;
        }
        else {
            return false;
        }
    }

    return Command::eventFilter(watched, event);
}

CommandRecentFiles::CommandRecentFiles(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Recent files"));
    this->setAction(action);
}

CommandRecentFiles::CommandRecentFiles(IAppContext* context, QMenu* containerMenu)
    : CommandRecentFiles(context)
{
    QObject::connect(containerMenu, &QMenu::aboutToShow, this, &CommandRecentFiles::recreateEntries);
}

void CommandRecentFiles::execute()
{
}

void CommandRecentFiles::recreateEntries()
{
    QMenu* menu = this->action()->menu();
    if (!menu)
        menu = new QMenu(this->widgetMain());

    menu->clear();
    int idFile = 0;
    auto appModule = AppModule::get();
    const RecentFiles& recentFiles = appModule->properties()->recentFiles;
    for (const RecentFile& recentFile : recentFiles) {
        const QString strFilePath = filepathTo<QString>(recentFile.filepath);
        const QString strEntryRecentFile = Command::tr("%1 | %2").arg(++idFile).arg(strFilePath);
        menu->addAction(strEntryRecentFile, this, [=]{
            FileCommandTools::openDocument(this->context(), recentFile.filepath);
        });
    }

    if (!recentFiles.empty()) {
        menu->addSeparator();
        menu->addAction(Command::tr("Clear menu"), this, [=]{
            menu->clear();
            appModule->properties()->recentFiles.setValue({});
        });
    }

    this->action()->setMenu(menu);
}

CommandImportInCurrentDocument::CommandImportInCurrentDocument(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Import"));
    action->setToolTip(Command::tr("Import in current document"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::Import));
    this->setAction(action);
}

void CommandImportInCurrentDocument::execute()
{
    const GuiDocument* guiDoc = this->currentGuiDocument();
    if (!guiDoc)
        return;

    const auto resFileNames = OpenFileNames::get(this->widgetMain());
    if (resFileNames.listFilepath.empty())
        return;

    FileCommandTools::importInDocument(this->context(), guiDoc->document(), resFileNames.listFilepath);
    for (const FilePath& fp : resFileNames.listFilepath)
        AppModule::get()->prependRecentFile(fp);
}

bool CommandImportInCurrentDocument::getEnabledStatus() const
{
    return this->app()->documentCount() != 0
           && this->context()->currentPage() == IAppContext::Page::Documents;
}

CommandExportSelectedApplicationItems::CommandExportSelectedApplicationItems(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Export selected items"));
    action->setToolTip(Command::tr("Export selected items"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::Export));
    this->setAction(action);
}

void CommandExportSelectedApplicationItems::execute()
{
    auto appModule = AppModule::get();
    if (this->guiApp()->selectionModel()->selectedItems().empty()) {
        appModule->emitError(Command::textIdTr("No item selected for export"));
        return;
    }

    QStringList listWriterFileFilter;
    for (IO::Format format : appModule->ioSystem()->writerFormats())
        listWriterFileFilter.append(fileFilter(format));

    auto lastSettings = ImportExportSettings::load();
    const QString strFilepath =
        QFileDialog::getSaveFileName(
            this->widgetMain(),
            Command::tr("Select Output File"),
            filepathTo<QString>(lastSettings.openDir),
            listWriterFileFilter.join(QLatin1String(";;")),
            &lastSettings.selectedFilter
        );
    if (strFilepath.isEmpty())
        return;

    lastSettings.openDir = filepathFrom(strFilepath);
    const IO::Format format = formatFromFilter(lastSettings.selectedFilter);
    const TaskId taskId = this->taskMgr()->newTask([=](TaskProgress* progress) {
        QElapsedTimer chrono;
        chrono.start();
        const bool okExport =
            appModule->ioSystem()->exportApplicationItems()
                .targetFile(filepathFrom(strFilepath))
                .targetFormat(format)
                .withItems(this->guiApp()->selectionModel()->selectedItems())
                .withParameters(appModule->findWriterParameters(format))
                .withMessenger(appModule)
                .withTaskProgress(progress)
            .execute();
        if (okExport)
            appModule->emitInfo(fmt::format(Command::textIdTr("Export time: {}ms"), chrono.elapsed()));
    });
    this->taskMgr()->setTitle(taskId, to_stdString(QFileInfo(strFilepath).fileName()));
    this->taskMgr()->run(taskId);
    ImportExportSettings::save(lastSettings);
}

bool CommandExportSelectedApplicationItems::getEnabledStatus() const
{
    return this->app()->documentCount() != 0
            && this->context()->currentPage() == IAppContext::Page::Documents;
}

CommandCloseCurrentDocument::CommandCloseCurrentDocument(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Close \"%1\""));
    action->setToolTip(action->text());
    action->setIcon(mayoTheme()->icon(Theme::Icon::Cross));
    action->setShortcut(Qt::CTRL | Qt::Key_W);
    this->setAction(action);

    QObject::connect(
        context, &IAppContext::currentDocumentChanged,
        this, &CommandCloseCurrentDocument::updateActionText
    );
    this->app()->signalDocumentNameChanged.connectSlot([=](const DocumentPtr& doc) {
        if (this->currentDocument() == doc->identifier())
            this->updateActionText(this->currentDocument());
    });

    this->updateActionText(-1);
}

void CommandCloseCurrentDocument::execute()
{
    FileCommandTools::closeDocument(this->context(), this->currentDocument());
}

bool CommandCloseCurrentDocument::getEnabledStatus() const
{
    return this->app()->documentCount() != 0;
}

void CommandCloseCurrentDocument::updateActionText(Document::Identifier docId)
{
    DocumentPtr docPtr = this->app()->findDocumentByIdentifier(docId);
    const QString docName = to_QString(docPtr ? docPtr->name() : std::string{});
    const QString textActionClose =
        docPtr ?
            Command::tr("Close %1").arg(strFilepathQuoted(docName)) :
            Command::tr("Close")
        ;
    this->action()->setText(textActionClose);
    this->action()->setToolTip(textActionClose);
}

CommandCloseAllDocuments::CommandCloseAllDocuments(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Close all"));
    action->setToolTip(Command::tr("Close all documents"));
    action->setShortcut((Qt::CTRL | Qt::SHIFT) | Qt::Key_W);
    this->setAction(action);
}

void CommandCloseAllDocuments::execute()
{
    while (!this->guiApp()->guiDocuments().empty())
        FileCommandTools::closeDocument(this->context(), this->currentDocument());
}

bool CommandCloseAllDocuments::getEnabledStatus() const
{
    return this->app()->documentCount() != 0;
}

CommandCloseAllDocumentsExceptCurrent::CommandCloseAllDocumentsExceptCurrent(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Close all except current"));
    action->setToolTip(Command::tr("Close all except current document"));
    this->setAction(action);

    QObject::connect(
        context, &IAppContext::currentDocumentChanged,
        this, &CommandCloseAllDocumentsExceptCurrent::updateActionText
    );
    this->app()->signalDocumentNameChanged.connectSlot([=](const DocumentPtr& doc) {
        if (this->currentDocument() == doc->identifier())
            this->updateActionText(this->currentDocument());
    });

    this->updateActionText(-1);
}

void CommandCloseAllDocumentsExceptCurrent::execute()
{
    GuiDocument* currentGuiDoc = this->currentGuiDocument();
    std::vector<GuiDocument*> vecGuiDoc;
    for (GuiDocument* guiDoc : this->guiApp()->guiDocuments())
        vecGuiDoc.push_back(guiDoc);

    for (GuiDocument* guiDoc : vecGuiDoc) {
        if (guiDoc != currentGuiDoc)
            FileCommandTools::closeDocument(this->context(), guiDoc->document()->identifier());
    }
}

bool CommandCloseAllDocumentsExceptCurrent::getEnabledStatus() const
{
    return this->app()->documentCount() != 0;
}

void CommandCloseAllDocumentsExceptCurrent::updateActionText(Document::Identifier docId)
{
    DocumentPtr docPtr = this->app()->findDocumentByIdentifier(docId);
    const QString docName = to_QString(docPtr ? docPtr->name() : std::string{});
    const QString textActionClose =
        docPtr ?
            Command::tr("Close all except %1").arg(strFilepathQuoted(docName)) :
            Command::tr("Close all except current")
        ;
    this->action()->setText(textActionClose);
}

CommandQuitApplication::CommandQuitApplication(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Quit"));
    this->setAction(action);
}

void CommandQuitApplication::execute()
{
    QApplication::quit();
}

} // namespace Mayo
