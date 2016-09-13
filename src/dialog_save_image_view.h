#pragma once

#include <QtWidgets/QDialog>

class Image_PixMap;

namespace Mayo {

class WidgetOccView;

class DialogSaveImageView : public QDialog
{
    Q_OBJECT

public:
    DialogSaveImageView(const WidgetOccView* view, QWidget *parent = nullptr);
    ~DialogSaveImageView();

private:
    void saveFile();
    void clipboardCopy();
    void preview();

    bool createImageView(Image_PixMap* img) const;

    class Ui_DialogSaveImageView* m_ui = nullptr;
    const WidgetOccView* m_view = nullptr;
};

} // namespace Mayo
