#pragma once

#include <QtWidgets/QMainWindow>

namespace Mayo {

class Application;
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
    void importPartFinished(
            bool ok, const QString& filepath, const QString& msg);

private:
    void newDoc();
    void openPartInNewDoc();
    void importPartInCurrentDoc();
    void quitApp();
    void editOptions();
    void saveImageView();
    void aboutMayo();
    void reportbug();

    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onApplicationTreeWidgetSelectionChanged();
    void onImportPartFinished(
            bool ok, const QString& filepath, const QString& msg);
    void onTabCloseRequested(int tabIndex);

    void updateControlsActivation();

    GuiApplication* m_guiApp = nullptr;
    class Ui_MainWindow* m_ui = nullptr;
};

} // namespace Mayo
