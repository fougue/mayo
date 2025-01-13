/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

class TestApp : public QObject {
    Q_OBJECT
private slots:
    void DocumentFilesWatcher_test();

    void FilePathConv_test();

    void QStringUtils_append_test();
    void QStringUtils_append_test_data();
    void QStringUtils_text_test();
    void QStringUtils_text_test_data();

    void RecentFiles_test();
    void RecentFiles_QPixmap_test();

    void AppUiState_test();

    void StringConv_test();

    void QtGuiUtils_test();
};

} // namespace Mayo
