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
    static void openDocument(IAppContext* context, FilePath fp);
};

class CommandNewDocument : public Command {
public:
    CommandNewDocument(IAppContext* context);
    void execute() override;
};

class CommandOpenDocuments : public Command {
public:
    CommandOpenDocuments(IAppContext* context);
    void execute() override;
    bool eventFilter(QObject* watched, QEvent* event) override;
};

class CommandRecentFiles : public Command {
public:
    CommandRecentFiles(IAppContext* context);
    CommandRecentFiles(IAppContext* context, QMenu* containerMenu);
    void execute() override;
    void recreateEntries();
};

class CommandImportInCurrentDocument : public Command {
public:
    CommandImportInCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;
};

class CommandExportSelectedApplicationItems : public Command {
public:
    CommandExportSelectedApplicationItems(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;
};

class CommandCloseCurrentDocument : public Command {
public:
    CommandCloseCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

private:
    void updateActionText(Document::Identifier docId);
};

class CommandCloseAllDocuments : public Command {
public:
    CommandCloseAllDocuments(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;
};

class CommandCloseAllDocumentsExceptCurrent : public Command {
public:
    CommandCloseAllDocumentsExceptCurrent(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

private:
    void updateActionText(Document::Identifier docId);
};

class CommandQuitApplication : public Command {
public:
    CommandQuitApplication(IAppContext* context);
    void execute() override;
};

} // namespace Mayo
