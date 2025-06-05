/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

namespace IO { class System; }

class TestIO : public QObject {
    Q_OBJECT
private slots:
    void IO_Reload_bugGitHub332_test();

    void IO_probeFormat_test();
    void IO_probeFormat_test_data();
    void IO_probeFormatDirect_test();
    void IO_OccStaticVariablesRollback_test();
    void IO_OccStaticVariablesRollback_test_data();
    void IO_bugGitHub166_test();
    void IO_bugGitHub166_test_data();
    void IO_bugGitHub258_test();

    void initTestCase();
    void cleanupTestCase();

private:
    IO::System* m_ioSystem = nullptr;
};

} // namespace Mayo
