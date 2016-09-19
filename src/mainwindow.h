#pragma once

#include "application.h"
#include <QtWidgets/QMainWindow>

namespace Mayo {

class Document;
class GuiApplication;
class GuiDocument;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(GuiApplication* guiApp, QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void operationFinished(bool ok, const QString& msg);

private:
    void newDoc();
    void openPartInNewDoc();
    void importPartInCurrentDoc();
    void exportSelectedItems();
    void quitApp();
    void editOptions();
    void saveImageView();
    void aboutMayo();
    void reportbug();

    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onApplicationTreeWidgetSelectionChanged();
    void onOperationFinished(bool ok, const QString& msg);
    void onTabCloseRequested(int tabIndex);

    void doExportSelectedItems(
            Application::PartFormat format,
            const Application::ExportOptions& opts,
            const QString& filepath);
    void updateControlsActivation();

    GuiApplication* m_guiApp = nullptr;
    class Ui_MainWindow* m_ui = nullptr;
};

} // namespace Mayo
