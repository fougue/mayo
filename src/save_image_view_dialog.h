#pragma once

#include <QtWidgets/QDialog>

class Image_PixMap;

namespace Mayo {

class QtOccView;

class SaveImageViewDialog : public QDialog
{
    Q_OBJECT

public:
    SaveImageViewDialog(const QtOccView* view, QWidget *parent = nullptr);
    ~SaveImageViewDialog();

private:
    void saveFile();
    void clipboardCopy();
    void preview();

    bool createImageView(Image_PixMap* img) const;

    class Ui_SaveImageViewDialog* m_ui = nullptr;
    const QtOccView* m_view = nullptr;
};

} // namespace Mayo
