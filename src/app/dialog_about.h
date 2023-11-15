/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QDialog>
#include <string_view>

namespace Mayo {

class DialogAbout : public QDialog {
    Q_OBJECT
public:
    DialogAbout(QWidget* parent = nullptr);
    ~DialogAbout();

    void addLibraryInfo(std::string_view libName, std::string_view libVersion);

private:
    class Ui_DialogAbout* m_ui = nullptr;
};

} // namespace Mayo
