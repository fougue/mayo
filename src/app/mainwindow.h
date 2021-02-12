/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property.h"
#include "../graphics/graphics_object_base_property_group.h"
#include <QtWidgets/QMainWindow>
#include <memory>
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

    void openDocumentsFromList(const QStringList& listFilePath);

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void currentDocumentIndexChanged(int docIdx);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void newDocument();
    void openDocuments();
    void importInCurrentDoc();
    void exportSelectedItems();
    void quitApp();
    void toggleCurrentDocOriginTrihedron();
    void zoomInCurrentDoc();
    void zoomOutCurrentDoc();
    void editOptions();
    void saveImageView();
    void inspectXde();
    void toggleFullscreen();
    void toggleLeftSidebar();
    void aboutMayo();
    void reportbug();

    void onApplicationItemSelectionChanged();
    void onOperationFinished(bool ok, const QString& msg);
    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onGuiDocumentErased(GuiDocument* guiDoc);
    void onWidgetFileSystemLocationActivated(const QFileInfo& loc);
    void onLeftContentsPageChanged(int pageId);
    void onCurrentDocumentIndexChanged(int idx);

    void closeCurrentDocument();
    void closeDocument(WidgetGuiDocument* widget);
    void closeDocument(int docIndex);
    void closeAllDocumentsExceptCurrent();
    void closeAllDocuments();

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
    Qt::WindowStates m_previousWindowState = Qt::WindowNoState;
    std::unique_ptr<PropertyGroupSignals> m_ptrCurrentNodeDataProperties;
    std::unique_ptr<GraphicsObjectBasePropertyGroup> m_ptrCurrentNodeGraphicsProperties;
};

} // namespace Mayo
