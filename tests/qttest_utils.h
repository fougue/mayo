/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtTest/QtTest>

// MAYO_QVERIFY_THROWS_EXCEPTION() is a helper macro to facilitate Qt5/Qt6 portability as
// Qt5 QVERIFY_EXCEPTION_THROWN() was deprecated since Qt6.3.0
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#  define MAYO_QVERIFY_THROWS_EXCEPTION(exceptionType, ...)  QVERIFY_EXCEPTION_THROWN(__VA_ARGS__, exceptionType)
#else
#  define MAYO_QVERIFY_THROWS_EXCEPTION(exceptionType, ...)  QVERIFY_THROWS_EXCEPTION(exceptionType, __VA_ARGS__)
#endif

// Backport of Qt 6.4's QCOMPARE_LT() macro for older Qt versions
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#  define QCOMPARE_LT(computed, baseline) \
    QVERIFY2((computed) < (baseline), qPrintable(QString("FAIL: %1 !< %2").arg(computed).arg(baseline)))
#endif
