#pragma once

#include <QtWidgets/QDialog>
class QToolButton;

namespace Mayo {

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog();

    void accept() override;

private:
    void chooseColor(
            const QColor& currentColor,
            QToolButton* targetBtn,
            QColor* targetColor);

    class Ui_OptionsDialog* m_ui = nullptr;
    QColor m_brepShapeDefaultColor;
    QColor m_meshDefaultColor;
};

} // namespace Mayo
