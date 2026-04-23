/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
