/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtWidgets/QDialog>
#include <string_view>

namespace Mayo {

class DialogAbout : public QDialog {
    Q_OBJECT
public:
    explicit DialogAbout(QWidget* parent = nullptr);
    ~DialogAbout();

    void addLibraryInfo(std::string_view libName, std::string_view libVersion);

private:
    class Ui_DialogAbout* m_ui = nullptr;
};

} // namespace Mayo
