#pragma once

#include "application.h"

#include <QtWidgets/QDialog>

namespace Mayo {

class DialogExportOptions : public QDialog
{
    Q_OBJECT

public:
    DialogExportOptions(QWidget *parent = nullptr);
    ~DialogExportOptions();

    Application::PartFormat partFormat() const;
    void setPartFormat(Application::PartFormat format);

    Application::ExportOptions currentExportOptions() const;

    void accept() override;

private:
    Application::PartFormat m_partFormat = Application::PartFormat::Unknown;
    class Ui_DialogExportOptions* m_ui = nullptr;
};

} // namespace Mayo
