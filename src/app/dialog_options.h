/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QDialog>
#include <functional>

namespace Mayo {

class Settings;

class DialogOptions : public QDialog {
    Q_OBJECT
public:
    DialogOptions(Settings* settings, QWidget *parent = nullptr);
    ~DialogOptions();

private:
    class Ui_DialogOptions* m_ui = nullptr;
};

} // namespace Mayo
