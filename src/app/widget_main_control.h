/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/filepath.h"
#include "../base/property.h"
#include "iwidget_main_page.h"

#include <memory>
#include <vector>
#include <unordered_set>

class QFileInfo;
class QMenu;

namespace Mayo {

class IAppContext;
class CommandContainer;
class DocumentFilesWatcher;
class DocumentTreeNode;
class GuiApplication;
class GuiDocument;
class ItemViewButtons;
class WidgetGuiDocument;

// Provides a main page to control opened documents in the application
// Comes with the model tree, 3D view associated to each document, ...
class WidgetMainControl : public IWidgetMainPage {
    Q_OBJECT
public:
    WidgetMainControl(GuiApplication* guiApp, QWidget* parent = nullptr);
    ~WidgetMainControl();

    void initialize(const CommandContainer* cmdContainer) override;
    void updatePageControlsActivation() override;

    // Widget at the left side of the app providing access to the model tree, file system, ...
    QWidget* widgetLeftSideBar() const;

    // Factor value must be in [0, 1]
    double widgetLeftSideBarWidthFactor() const;
    void setWidgetLeftSideBarWidthFactor(double factor);

    int widgetGuiDocumentCount() const;
    WidgetGuiDocument* widgetGuiDocument(int idx) const;
    WidgetGuiDocument* currentWidgetGuiDocument() const;
    int indexOfWidgetGuiDocument(WidgetGuiDocument* widgetDoc) const;
    void removeWidgetGuiDocument(WidgetGuiDocument* widgetDoc);

    int currentDocumentIndex() const;
    void setCurrentDocumentIndex(int idx);

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void currentDocumentIndexChanged(int docIndex);

private:
    QMenu* createMenuModelTreeSettings();

    void editDocumentTreeNode(const DocumentTreeNode& docTreeNode);

    void onApplicationItemSelectionChanged();
    void onLeftContentsPageChanged(int pageId);
    void onWidgetFileSystemLocationActivated(const QFileInfo& loc);

    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onGuiDocumentErased(GuiDocument* guiDoc);

    void onCurrentDocumentIndexChanged(int idx);
    void onDocumentFileChanged(const DocumentPtr& doc);
    void onDocumentFilePathChanged(const DocumentPtr& doc, const FilePath& fp);
    void onSplitterMainMoved(int pos, int index);

    QWidget* findLeftHeaderPlaceHolder() const;
    QWidget* recreateLeftHeaderPlaceHolder();

    void reloadDocumentAfterChange(const DocumentPtr& doc);

    class Ui_WidgetMainControl* m_ui = nullptr;
    GuiApplication* m_guiApp = nullptr;
    IAppContext* m_appContext = nullptr;
    ItemViewButtons* m_listViewBtns = nullptr;
    std::vector<std::unique_ptr<PropertyGroup>> m_ptrCurrentNodeProperties;
    DocumentFilesWatcher* m_docFilesWatcher = nullptr;
    std::unordered_set<DocumentPtr> m_pendingDocsToReload;
    double m_widgetLeftSideBarWidthFactor = 0.25;
};

} // namespace Mayo
