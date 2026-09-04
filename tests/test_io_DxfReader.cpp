/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_io.h"

#include "../src/base/application.h"
#include "../src/base/io_system.h"
#include "../src/io_dxf/io_dxf.h"

#include <QtTest/QtTest>

#include <string>

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(std::string)

namespace Mayo {

void TestIO::DxfReader_replaceTextControlCodes_test()
{
    QFETCH(std::string, strInput);
    QFETCH(std::string, strOutput);

    IO::DxfReader::replaceTextControlCodes(&strInput);
    QCOMPARE(strInput, strOutput);
}

void TestIO::DxfReader_replaceTextControlCodes_test_data()
{
    QTest::addColumn<std::string>("strInput");
    QTest::addColumn<std::string>("strOutput");

    using namespace std::string_literals;
    QTest::newRow("test1") << "Temperature: 37%%dC  Diameter: %%c45mm"s << "Temperature: 37°C  Diameter: Ø45mm"s;
    QTest::newRow("test2") << "Percent %%%"s << "Percent %"s;
    QTest::newRow("test3") << "%%d%%c%%p%%%%%o%%u%%k"s << "°Ø±%"s;
    QTest::newRow("test4") << "char %%"s << "char %%"s;
    QTest::newRow("test5") << "%%"s << "%%"s;
    QTest::newRow("test6") << "Weird %%1 %%12 %%200 %%245 %%45589"s << "Weird %%1 %%12 ? ? ?89"s;
    QTest::newRow("test7") << "%%oText%%o and %%uother%%u"s << "Text and other"s;
    QTest::newRow("test8") << "%%D%%C%%P%%%%%O%%U%%K"s << "°Ø±%"s;
    QTest::newRow("test9") << "Unsupported:\"%%a %%!\" OK:%%p"s << "Unsupported:\"%%a %%!\" OK:±"s;
    QTest::newRow("test10") << "abc %% "s << "abc %% "s;
    QTest::newRow("test11") << "%%12Z%%12Z"s << "%%12Z%%12Z"s;
    QTest::newRow("test12") << "%%d abc %%uDEF%%u GHI %%p"s << "° abc DEF GHI ±"s;
    QTest::newRow("test13") << "%%c中文%%c"s << "Ø中文Ø"s;
    QTest::newRow("test14") << "%%c%%uX"s << "ØX"s;
    QTest::newRow("test15") << "%%cé%%c"s << "ØéØ"s;
    QTest::newRow("test16") << "%%o%%c"s << "Ø"s;
}

void TestIO::DxfReader_getPlainMText_test()
{
    QFETCH(std::string, strInput);
    QFETCH(std::string, strOutput);

    const std::string strActual = IO::DxfReader::getPlainMText(strInput);
    QCOMPARE(strActual, strOutput);
}

void TestIO::DxfReader_getPlainMText_test_data()
{
    QTest::addColumn<std::string>("strInput");
    QTest::addColumn<std::string>("strOutput");

    using namespace std::string_literals;
    QTest::newRow("basic-braces-H-W-P")
        << "{ \\H1.5x;Big }\\P Normal \\W0.8;Small"s
        << "Big\n Normal Small"s;

    QTest::newRow("underline-overscore")
        << "\\LUnder\\l line \\OOver\\o line"s
        << "Under line Over line"s;

    //QTest::newRow("stacked-simple")
    //    << "\\S10^3; et \\S1/2; et \\SA ^ B ;"
    //    << "10/3 et 1/2 et A/B";

    QTest::newRow("colors-aci-rgb")
        << "\\C1;Red \\c16711680;RedRGB"s
        << "Red RedRGB"s;

    QTest::newRow("font-switch")
        << "\\fNoto Sans|b1;Bold text \\fArial|b0;fine"s
        << "Bold text fine"s;

    QTest::newRow("unicode-uplus")
        << "Deg: \\U+00B0 Dia: \\U+00D8 Breve: \\U+0103"s
        << "Deg: ° Dia: Ø Breve: ă"s;

    QTest::newRow("text-control-codes")
        << "T=37%%dC  D=%%c45  Tol=%%p  Percent=%%%"s
        << "T=37°C  D=Ø45  Tol=±  Percent=%"s;

    QTest::newRow("tab-stops")
        << "Before \\pt0.24,17;X and \\pxt0.24,17;Y After"s
        << "Before X and Y After"s;

    //QTest::newRow("optional-semicolon")
    //    << "\\A2Align haut \\C7Blanc \\c255RGB \\H2xGrand"
    //    << "Align haut Blanc RGB Grand";

    QTest::newRow("unknown-seqs")
        << "Keep \\Zhere and { } keep \\\\X too"s
        << "Keep \\Zhere and  keep \\\\X too"s;

    //QTest::newRow("unicode-surrogate")
    //    << "Bad \\U+D800 ok"
    //    << "Bad ? ok";

    QTest::newRow("caret-codes_0") << "A^IB^JC^MZ"s << "A\tB\nCZ"s;
    QTest::newRow("caret-codes_1") << "X^IY^JZ^M."s << "X\tY\nZ."s;

    QTest::newRow("unicode-plus")
        << "A\\U+00B0B \\U+00D8 C \\U+0103"s
        << "A°B Ø C ă"s;

    QTest::newRow("unicode-unchanged") << "X \\U+010 Y"s << "X \\U+010 Y"s;
}

void TestIO::DxfReader_lwPolylineClosedDuplicateLastVertex_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    const bool okImport = m_ioSystem->importInDocument(
        doc, "tests/inputs/lwpolyline_closed_duplicate_last_vertex.dxf"
    );
    QVERIFY(okImport);
    QCOMPARE(doc->entityCount(), 1);
}

} // namespace Mayo
