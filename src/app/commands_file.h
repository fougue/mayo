/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "commands_api.h"

#include <gsl/span>

namespace Mayo {

class FileCommandTools {
public:
    static void closeDocument(IAppContext* context, Document::Identifier docId);
    static void closeAllDocuments(IAppContext* context);

    static void openDocumentsFromList(IAppContext* context, gsl::span<const FilePath> listFilePath);
    static void openDocument(IAppContext* context, const FilePath& filePath);

    static void importInDocument(
        IAppContext* context,
        const DocumentPtr& targetDoc,
        gsl::span<const FilePath> listFilePaths
    );
    static void importInDocument(
        IAppContext* context,
        const DocumentPtr& targetDoc,
        const FilePath& filePath
    );
};

class CommandNewDocument : public Command {
public:
    explicit CommandNewDocument(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "new-doc";
};

class CommandOpenDocuments : public Command {
public:
    explicit CommandOpenDocuments(IAppContext* context);
    void execute() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    static constexpr std::string_view Name = "open-docs";
};

class CommandRecentFiles : public Command {
public:
    explicit CommandRecentFiles(IAppContext* context);
    CommandRecentFiles(IAppContext* context, QMenu* containerMenu);
    void execute() override;
    void recreateEntries();

    static constexpr std::string_view Name = "recent-files";
};

class CommandImportInCurrentDocument : public Command {
public:
    explicit CommandImportInCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "import";
};

class CommandExportSelectedApplicationItems : public Command {
public:
    explicit CommandExportSelectedApplicationItems(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "export";
};

class CommandCloseCurrentDocument : public Command {
public:
    explicit CommandCloseCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "close-doc";

private:
    void updateActionText(Document::Identifier docId);
};

class CommandCloseAllDocuments : public Command {
public:
    explicit CommandCloseAllDocuments(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "close-all-docs";
};

class CommandCloseAllDocumentsExceptCurrent : public Command {
public:
    explicit CommandCloseAllDocumentsExceptCurrent(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "close-all-docs-except-current";

private:
    void updateActionText(Document::Identifier docId);
};

class CommandQuitApplication : public Command {
public:
    explicit CommandQuitApplication(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "quit-app";
};

} // namespace Mayo
