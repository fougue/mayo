/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application.h"

#include <QtWidgets/QDialog>

namespace Mayo {

#ifdef HAVE_GMIO
class DialogExportOptions : public QDialog {
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
#endif

} // namespace Mayo
