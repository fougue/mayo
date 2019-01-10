/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QDialog>
class QToolButton;

namespace Mayo {

class DialogOptions : public QDialog {
    Q_OBJECT
public:
    DialogOptions(QWidget *parent = nullptr);
    ~DialogOptions();

    void accept() override;

private:
    void chooseColor(
            const QColor& currentColor,
            QToolButton* targetBtn,
            QColor* targetColor);

    class Ui_DialogOptions* m_ui = nullptr;
    QColor m_brepShapeDefaultColor;
    QColor m_meshDefaultColor;
};

} // namespace Mayo
