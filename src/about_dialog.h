#pragma once

#include <QtWidgets/QDialog>

namespace Mayo {

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    class Ui_AboutDialog *m_ui;
};

} // namespace Mayo
