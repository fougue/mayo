/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include "application.h"
#include <QtWidgets/QMainWindow>
#include <functional>
class QFileInfo;

namespace Mayo {

class Document;
class GuiApplication;
class GuiDocument;
class WidgetGuiDocument;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(GuiApplication* guiApp, QWidget* parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void operationFinished(bool ok, const QString& msg);
    void currentDocumentIndexChanged(int docIdx);

private:
    void newDoc();
    void openPartInNewDoc();
    void importInCurrentDoc();
    void exportSelectedItems();
    void quitApp();
    void editOptions();
    void saveImageView();
    void inspectXde();
    void toggleFullscreen();
    void toggleLeftSidebar();
    void aboutMayo();
    void reportbug();

    void onApplicationTreeWidgetSelectionChanged();
    void onOperationFinished(bool ok, const QString& msg);
    void onHomePageLinkActivated(const QString& link);
    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onWidgetFileSystemLocationActivated(const QFileInfo& loc);
    void onLeftContentsPageChanged(int pageId);
    void closeCurrentDocument();
    void closeDocument(int docIndex);

    void foreachOpenFileName(
            std::function<void (Application::PartFormat, QString)>&& func);
    void runImportTask(
            Document* doc,
            Application::PartFormat format,
            const QString& filepath);
    void runExportTask(
            const std::vector<DocumentItem*>& docItems,
            Application::PartFormat format,
            const Application::ExportOptions& opts,
            const QString& filepath);

    void updateControlsActivation();
    void updateActionText(QAction* action);

    int currentDocumentIndex() const;
    void setCurrentDocumentIndex(int idx);

    WidgetGuiDocument* widgetGuiDocument(int idx) const;

    GuiApplication* m_guiApp = nullptr;
    class Ui_MainWindow* m_ui = nullptr;
    Qt::WindowStates m_previousWindowState = Qt::WindowNoState;
};

} // namespace Mayo
