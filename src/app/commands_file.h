/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/span.h"
#include "commands_api.h"

namespace Mayo {

class FileCommandTools {
public:
    static void closeDocument(IAppContext* context, Document::Identifier docId);
    static void openDocumentsFromList(IAppContext* context, Span<const FilePath> listFilePath);
    static void openDocument(IAppContext* context, const FilePath& filePath);
    static void importInDocument(
        IAppContext* context,
        const DocumentPtr& targetDoc,
        Span<const FilePath> listFilePaths
    );
    static void importInDocument(
        IAppContext* context,
        const DocumentPtr& targetDoc,
        const FilePath& filePath
    );
};

class CommandNewDocument : public Command {
public:
    CommandNewDocument(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "new-doc";
};

class CommandOpenDocuments : public Command {
public:
    CommandOpenDocuments(IAppContext* context);
    void execute() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    static constexpr std::string_view Name = "open-docs";
};

class CommandRecentFiles : public Command {
public:
    CommandRecentFiles(IAppContext* context);
    CommandRecentFiles(IAppContext* context, QMenu* containerMenu);
    void execute() override;
    void recreateEntries();

    static constexpr std::string_view Name = "recent-files";
};

class CommandImportInCurrentDocument : public Command {
public:
    CommandImportInCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "import";
};

class CommandExportSelectedApplicationItems : public Command {
public:
    CommandExportSelectedApplicationItems(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "export";
};

class CommandCloseCurrentDocument : public Command {
public:
    CommandCloseCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "close-doc";

private:
    void updateActionText(Document::Identifier docId);
};

class CommandCloseAllDocuments : public Command {
public:
    CommandCloseAllDocuments(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "close-all-docs";
};

class CommandCloseAllDocumentsExceptCurrent : public Command {
public:
    CommandCloseAllDocumentsExceptCurrent(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "close-all-docs-except-current";

private:
    void updateActionText(Document::Identifier docId);
};

class CommandQuitApplication : public Command {
public:
    CommandQuitApplication(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "quit-app";
};

} // namespace Mayo
