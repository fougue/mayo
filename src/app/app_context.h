/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_api.h"

namespace Mayo {

class MainWindow;
class WidgetGuiDocument;

// Provides implementation of IAppContext based on MainWindow
class AppContext : public IAppContext {
public:
    AppContext(MainWindow* wnd);

    GuiApplication* guiApp() const override;
    TaskManager* taskMgr() const override;
    V3dViewController* v3dViewController(const GuiDocument* guiDoc) const override;

    QWidget* widgetMain() const override;
    QWidget* widgetPage(Page page) const override;
    Page currentPage() const override;
    void setCurrentPage(Page page) override;

    int findDocumentIndex(Document::Identifier docId) const override;
    Document::Identifier findDocumentFromIndex(int index) const override;

    Document::Identifier currentDocument() const override;
    void setCurrentDocument(Document::Identifier docId) override;

    void updateControlsEnabledStatus() override;

private:
    GuiDocument* guiDocument(int idx) const;
    WidgetGuiDocument* widgetGuiDocument(int idx) const;
    WidgetGuiDocument* findWidgetGuiDocument(std::function<bool(WidgetGuiDocument*)> fn) const;

    void onCurrentDocumentIndexChanged(int docIndex);

    MainWindow* m_wnd = nullptr;
};

} // namespace Mayo
