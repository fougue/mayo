/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
    QWidget* widgetMain() const override;
    QWidget* widgetLeftSidebar() const override;
    ModeWidgetMain modeWidgetMain() const override;
    V3dViewController* v3dViewController(const GuiDocument* guiDoc) const override;

    int findDocumentIndex(Document::Identifier docId) const override;
    Document::Identifier findDocumentFromIndex(int index) const override;

    Document::Identifier currentDocument() const override;
    void setCurrentDocument(Document::Identifier docId) override;

    void updateControlsEnabledStatus() override;
    void deleteDocumentWidget(const DocumentPtr& doc) override;

private:
    WidgetGuiDocument* findWidgetGuiDocument(std::function<bool(WidgetGuiDocument*)> fn) const;

    void onCurrentDocumentIndexChanged(int docIndex);

    MainWindow* m_wnd = nullptr;
};

} // namespace Mayo
