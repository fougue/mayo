/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "../base/property.h"
#include "../base/text_id.h"
#include <QtWidgets/QMainWindow>
#include <memory>
class QFileInfo;

namespace Mayo {

class GuiApplication;
class GuiDocument;
class TaskManager;
class WidgetGuiDocument;

class MainWindow : public QMainWindow {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::MainWindow)
public:
    MainWindow(GuiApplication* guiApp, QWidget* parent = nullptr);
    ~MainWindow();

    void openDocument(const FilePath& fp);
    void openDocumentsFromList(Span<const FilePath> listFilePath);

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void currentDocumentIndexChanged(int docIdx);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    // -- File menu
    void newDocument();
    void openDocuments();
    void importInCurrentDoc();
    void exportSelectedItems();
    void closeCurrentDocument();
    void closeAllDocumentsExceptCurrent();
    void closeAllDocuments();
    void quitApp();
    // -- Display menu
    void toggleCurrentDocOriginTrihedron();
    void toggleCurrentDocPerformanceStats();
    void zoomInCurrentDoc();
    void zoomOutCurrentDoc();
    // -- Tools menu
    void editOptions();
    void saveImageView();
    void inspectXde();
    // -- Window menu
    void toggleFullscreen();
    void toggleLeftSidebar();
    // -- Help menu
    void aboutMayo();
    void reportbug();

    void onApplicationItemSelectionChanged();
    void onOperationFinished(bool ok, const QString& msg);
    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onWidgetFileSystemLocationActivated(const QFileInfo& loc);
    void onLeftContentsPageChanged(int pageId);
    void onCurrentDocumentIndexChanged(int idx);

    void closeDocument(WidgetGuiDocument* widget);
    void closeDocument(int docIndex);

    void updateControlsActivation();

    int currentDocumentIndex() const;
    void setCurrentDocumentIndex(int idx);

    WidgetGuiDocument* widgetGuiDocument(int idx) const;
    WidgetGuiDocument* currentWidgetGuiDocument() const;
    QWidget* findLeftHeaderPlaceHolder() const;
    QWidget* recreateLeftHeaderPlaceHolder();
    QMenu* createMenuModelTreeSettings();
    QMenu* createMenuRecentFiles();
    QMenu* createMenuDisplayMode();

    GuiApplication* m_guiApp = nullptr;
    class Ui_MainWindow* m_ui = nullptr;
    TaskManager* m_taskMgr = nullptr;
    Qt::WindowStates m_previousWindowState = Qt::WindowNoState;
    std::unique_ptr<PropertyGroup> m_ptrCurrentNodeDataProperties;
    std::unique_ptr<PropertyGroupSignals> m_ptrCurrentNodeGraphicsProperties;
};

} // namespace Mayo
