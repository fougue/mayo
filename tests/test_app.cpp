/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

// Avoid MSVC conflicts with M_E, M_LOG2, ...
#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#  define _USE_MATH_DEFINES
#endif

#include "test_app.h"

#include "../src/app/app_module.h"
#include "../src/app/document_files_watcher.h"
#include "../src/app/qstring_utils.h"
#include "../src/app/qtgui_utils.h"
#include "../src/app/recent_files.h"
#include "../src/app/theme.h"
#include "../src/base/application.h"
#include "../src/base/document.h"
#include "../src/qtcommon/filepath_conv.h"
#include "../src/qtcommon/qstring_conv.h"

#include <QtCore/QtDebug>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QTemporaryFile>
#include <QtCore/QVariant>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtTest/QSignalSpy>

namespace Mayo {

namespace {

QPixmap createColorPixmap(const QColor& color, const QSize& size = QSize(64, 64))
{
    QPixmap pix(size);
    QPainter painter(&pix);
    painter.fillRect(0, 0, size.width(), size.height(), color);
    return pix;
}

FilePath temporaryFilePath()
{
    QTemporaryFile file;
    file.open();
    return filepathFrom(QFileInfo(file));
}

RecentFile createRecentFile(const QPixmap& thumbnail)
{
    QTemporaryFile file;
    file.open();
    RecentFile rf;
    rf.filepath = filepathFrom(QFileInfo(file));
    rf.thumbnailTimestamp = RecentFile::timestampLastModified(rf.filepath);
    rf.thumbnail.imageData = QtGuiUtils::toQByteArray(thumbnail);
    rf.thumbnail.imageCacheKey = thumbnail.cacheKey();
    return rf;
}

} // namespace

void TestApp::DocumentFilesWatcher_test()
{
    auto app = makeOccHandle<Application>();

    DocumentFilesWatcher docFilesWatcher(app);
    docFilesWatcher.enable(true);
    Document::Identifier changedDocId = -1;
    docFilesWatcher.signalDocumentFileChanged.connectSlot([&](DocumentPtr changedDoc) {
        changedDocId = changedDoc->identifier();
    });

    const FilePath cadFilePath = "tests/outputs/temp-cube.ply";
    auto fnCopyCadFile = [=]{
        std_filesystem::copy_file(
            "tests/inputs/cube.ply",
            cadFilePath,
            std_filesystem::copy_options::overwrite_existing
        );
    };

    fnCopyCadFile();
    DocumentPtr doc = app->newDocument();
    doc->setFilePath(cadFilePath);

    // Check file change on document is caught
    fnCopyCadFile();
    const bool okWait = QTest::qWaitFor([&]{ return changedDocId != -1; });
    QVERIFY(okWait);
    QCOMPARE(changedDocId, doc->identifier());

    // Check further file changes are not monitored until last one is acknowledged
    changedDocId = -1;
    fnCopyCadFile();
    QTest::qWait(125/*ms*/);
    QCOMPARE(changedDocId, -1);

    // Check closing document unmonitors file in DocumentFilesWatcher
    docFilesWatcher.acknowledgeDocumentFileChange(doc);
    app->closeDocument(doc);
    fnCopyCadFile();
    QTest::qWait(125/*ms*/);
    QCOMPARE(changedDocId, -1);
}

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
    RecentFiles recentFiles;
    recentFiles.push_back(createRecentFile(createColorPixmap(Qt::blue)));
    recentFiles.push_back(createRecentFile(createColorPixmap(Qt::white)));
    recentFiles.push_back(createRecentFile(createColorPixmap(Qt::red)));

    RecentFiles recentFiles_read;
    {
        QByteArray data;
        QDataStream wstream(&data, QIODevice::WriteOnly);
        AppModule::write(wstream, recentFiles);
        QDataStream rstream(&data, QIODevice::ReadOnly);
        AppModule::read(rstream, &recentFiles_read);
    }

    QCOMPARE(recentFiles.size(), recentFiles_read.size());
    for (RecentFiles::size_type i = 0; i < recentFiles.size(); ++i) {
        const RecentFile& lhs = recentFiles.at(i);
        const RecentFile& rhs = recentFiles_read.at(i);
        QCOMPARE(lhs.filepath, rhs.filepath);
        QVERIFY(lhs.thumbnailTimestamp != -1);
        QCOMPARE(lhs.thumbnailTimestamp, rhs.thumbnailTimestamp);
        QCOMPARE(lhs.thumbnail.imageData.size(), rhs.thumbnail.imageData.size());
        const QPixmap lhsPix = QtGuiUtils::toQPixmap(lhs.thumbnail.imageData);
        const QPixmap rhsPix = QtGuiUtils::toQPixmap(rhs.thumbnail.imageData);
        const QImage lhsImg = lhsPix.toImage();
        const QImage rhsImg = rhsPix.toImage();
        for (int i = 0; i < lhsPix.width(); ++i) {
            for (int j = 0; j < lhsPix.height(); ++j) {
                QCOMPARE(lhsImg.pixel(i, j), rhsImg.pixel(i, j));
            }
        } // endfor
    }
}

void TestApp::RecentFiles_QPixmap_test()
{
    QByteArray bytes;
    {
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << uint32_t(1);
        stream << filepathTo<QString>(temporaryFilePath());
        stream << QPixmap();//createColorPixmap(Qt::blue);
        stream << qint64(1);
    }

    {
        RecentFiles recentFiles;
        QDataStream stream(bytes);
        // Should not crash
        AppModule::read(stream, &recentFiles);
    }
}

void TestApp::StringConv_test()
{
    const QString text = "test_éç²µ§_测试_Тест";
    QCOMPARE(to_QString(string_conv<TCollection_AsciiString>(text)), text);
    QCOMPARE(to_QString(to_OccHandleHAsciiString(text)), text);
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
