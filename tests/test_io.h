/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
