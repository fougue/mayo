#pragma once

#include <QtWidgets/QMainWindow>

namespace Mayo {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void newDoc();
    void openPartInNewDoc();
    void importPartInCurrentDoc();
    void quitApp();

    void onTabCloseRequested(int tabIndex);

    class Ui_MainWindow *m_ui = nullptr;
};

} // namespace Mayo
