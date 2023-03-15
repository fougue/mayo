/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"
#include <QtWidgets/QDialog>
#include <V3d_View.hxx>

class Image_PixMap;

namespace Mayo {

class DialogSaveImageView : public QDialog {
    Q_OBJECT
public:
    DialogSaveImageView(const OccHandle<V3d_View>& view, QWidget* parent = nullptr);
    ~DialogSaveImageView();

private:
    void saveFile();
    void clipboardCopy();
    void preview();

    bool createImageView(Image_PixMap* img) const;

    class Ui_DialogSaveImageView* m_ui = nullptr;
    OccHandle<V3d_View> m_view;
};

} // namespace Mayo
