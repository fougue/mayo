/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

// Avoid MSVC conflicts with M_E, M_LOG2, ...
#ifdef _MSC_VER
#  define _USE_MATH_DEFINES
#endif

#include "test_app.h"

#include "../src/app/filepath_conv.h"
#include "../src/app/qstring_conv.h"
#include "../src/app/qstring_utils.h"
#include "../src/app/recent_files.h"
#include "../src/app/theme.h"
#include "../src/io_occ/io_occ.h"
#include "../src/gui/qtgui_utils.h"

#include <QtCore/QtDebug>
#include <QtCore/QFile>
#include <QtCore/QTemporaryFile>
#include <QtCore/QVariant>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtTest/QSignalSpy>

namespace Mayo {

void TestApp::FilePathConv_test()
{
    const char strTestPath[] = "../as1-oc-214 - 測試文件.stp";

    {
        const QByteArray testPathQt = strTestPath;
        const FilePath testPath = filepathFrom(testPathQt);
        QCOMPARE(filepathTo<QByteArray>(testPath), testPathQt);
    }

    {
        const QString testPathQt = strTestPath;
        const FilePath testPath = filepathFrom(testPathQt);
        QCOMPARE(filepathTo<QString>(testPath), testPathQt);
    }
}

void TestApp::QStringUtils_append_test()
{
    QFETCH(QString, strExpected);
    QFETCH(QString, str1);
    QFETCH(QString, str2);
    QFETCH(QLocale, locale);

    QString strActual = str1;
    QStringUtils::append(&strActual, str2, locale);
    QCOMPARE(strActual, strExpected);
}

void TestApp::QStringUtils_append_test_data()
{
    QTest::addColumn<QString>("strExpected");
    QTest::addColumn<QString>("str1");
    QTest::addColumn<QString>("str2");
    QTest::addColumn<QLocale>("locale");

    QTest::newRow("locale_c")
            << QString("1234")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale::c();
    QTest::newRow("locale_fr")
            << QString("1234")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale(QLocale::French);
    QTest::newRow("locale_arabic")
            << QString("3412")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale(QLocale::Arabic);
    QTest::newRow("locale_syriac")
            << QString("3412")
            << QStringLiteral("12")
            << QStringLiteral("34")
            << QLocale(QLocale::Syriac);
}

void TestApp::QStringUtils_text_test()
{
    QFETCH(QString, strActual);
    QFETCH(QString, strExpected);
    QCOMPARE(strActual, strExpected);
}

void TestApp::QStringUtils_text_test_data()
{
    QTest::addColumn<QString>("strActual");
    QTest::addColumn<QString>("strExpected");

    const QStringUtils::TextOptions opts_c_si_2 = { QLocale::c(), UnitSystem::SI, 2 };
    const QStringUtils::TextOptions opts_fr_si_2 = {
        QLocale(QLocale::French, QLocale::France), UnitSystem::SI, 2 };
    QTest::newRow("c_0.1")
            << QStringUtils::text(0.1, opts_c_si_2)
            << QStringLiteral("0.1");
    QTest::newRow("c_0.155")
            << QStringUtils::text(0.155, opts_c_si_2)
            << QStringLiteral("0.15");
    QTest::newRow("c_0.159")
            << QStringUtils::text(0.159, opts_c_si_2)
            << QStringLiteral("0.16");
    QTest::newRow("fr_1.4995")
            << QStringUtils::text(1.4995, opts_fr_si_2)
            << QStringLiteral("1,5");
    QTest::newRow("c_pnt0.55,4.8977,15.1445")
            << QStringUtils::text(gp_Pnt(0.55, 4.8977, 15.1445), opts_c_si_2)
            << QStringLiteral("(0.55mm 4.9mm 15.14mm)");
}

void TestApp::RecentFiles_test()
{
    auto fnColorPixmap = [](const QColor& color) {
        QPixmap pix(64, 64);
        QPainter painter(&pix);
        painter.fillRect(0, 0, 64, 64, color);
        return pix;
    };

    auto fnCreateRecentFile = [](const QPixmap& thumbnail) {
        QTemporaryFile file;
        file.open();
        RecentFile rf;
        rf.filepath = filepathFrom(QFileInfo(file));
        rf.thumbnailTimestamp = RecentFile::timestampLastModified(rf.filepath);
        rf.thumbnail = thumbnail;
        return rf;
    };

    RecentFiles recentFiles;
    recentFiles.push_back(fnCreateRecentFile(fnColorPixmap(Qt::blue)));
    recentFiles.push_back(fnCreateRecentFile(fnColorPixmap(Qt::white)));
    recentFiles.push_back(fnCreateRecentFile(fnColorPixmap(Qt::red)));

    RecentFiles recentFiles_read;
    {
        QByteArray data;
        QDataStream wstream(&data, QIODevice::WriteOnly);
        wstream << recentFiles;
        QDataStream rstream(&data, QIODevice::ReadOnly);
        rstream >> recentFiles_read;
    }

    QCOMPARE(recentFiles.size(), recentFiles_read.size());
    for (RecentFiles::size_type i = 0; i < recentFiles.size(); ++i) {
        const RecentFile& lhs = recentFiles.at(i);
        const RecentFile& rhs = recentFiles_read.at(i);
        QCOMPARE(lhs.filepath, rhs.filepath);
        QVERIFY(lhs.thumbnailTimestamp != -1);
        QCOMPARE(lhs.thumbnailTimestamp, rhs.thumbnailTimestamp);
        QCOMPARE(lhs.thumbnail.size(), rhs.thumbnail.size());
        const QImage lhsImg = lhs.thumbnail.toImage();
        const QImage rhsImg = rhs.thumbnail.toImage();
        for (int i = 0; i < lhs.thumbnail.width(); ++i) {
            for (int j = 0; j < lhs.thumbnail.height(); ++j) {
                QCOMPARE(lhsImg.pixel(i, j), rhsImg.pixel(i, j));
            }
        } // endfor
    }
}

void TestApp::StringConv_test()
{
    const QString text = "test_éç²µ§_测试_Тест";
    QCOMPARE(to_QString(string_conv<TCollection_AsciiString>(text)), text);
    QCOMPARE(to_QString(string_conv<Handle(TCollection_HAsciiString)>(text)), text);
    QCOMPARE(to_QString(to_stdString(text)), text);
    QCOMPARE(to_QString(to_OccExtString(text)), text);
}

void TestApp::QtGuiUtils_test()
{
    const QColor qtColor(51, 75, 128);
    const QColor qtColorA(51, 75, 128, 87);
    auto occColor = QtGuiUtils::toColor<Quantity_Color>(qtColor);
    auto occColorA = QtGuiUtils::toColor<Quantity_ColorRGBA>(qtColorA);
    QCOMPARE(QtGuiUtils::toQColor(occColor), qtColor);
    QCOMPARE(QtGuiUtils::toQColor(occColorA), qtColorA);
}

} // namespace Mayo
