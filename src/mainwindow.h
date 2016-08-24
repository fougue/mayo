#pragma once

#include <QtWidgets/QMainWindow>

namespace Mayo {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
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
    void aboutMayo();
    void reportbug();

    void onImportPartFinished(
            bool ok, const QString& filepath, const QString& msg);
    void onTabCloseRequested(int tabIndex);

    void updateControlsActivation();

    class Ui_MainWindow* m_ui = nullptr;
};

} // namespace Mayo
