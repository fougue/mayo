/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_io.h"

#include "../src/base/application.h"
#include "../src/base/caf_utils.h"
#include "../src/base/io_system.h"
#include "../src/base/property_value_conversion.h"
#include "../src/base/occ_static_variables_rollback.h"
#include "../src/base/string_conv.h"
#include "../src/io_dxf/io_dxf.h"
#include "../src/io_occ/io_occ.h"
#include "../src/io_off/io_off_reader.h"
#include "../src/io_off/io_off_writer.h"
#include "../src/io_ply/io_ply_reader.h"
#include "../src/io_ply/io_ply_writer.h"
#include <common/mayo_config.h>

#include <BRep_Tool.hxx>
#include <Interface_ParamType.hxx>
#include <Interface_Static.hxx>
#include <TopoDS.hxx>

#include <QtTest/QtTest>

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(Mayo::IO::Format)
Q_DECLARE_METATYPE(Mayo::PropertyValueConversion::Variant)
Q_DECLARE_METATYPE(std::string)

namespace Mayo {

void TestIO::Regression_bugGitHub332_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    // Import file in document
    const FilePath docFilePath = "tests/inputs/#332_file.stp";
    {
        const bool okImport = m_ioSystem->importInDocument(doc, docFilePath);
        QVERIFY(okImport);
        QCOMPARE(doc->entityCount(), 1);
        const TDF_Label entityLabel = doc->firstEntityNodeLabel();
        QCOMPARE(to_stdString(CafUtils::labelAttrStdName(entityLabel)), std::string{"Root"});
        QVERIFY(XCaf::isShapeAssembly(entityLabel));
        QCOMPARE(XCaf::shapeComponents(entityLabel).Size(), 3);
        for (const TDF_Label& componentLabel : XCaf::shapeComponents(entityLabel)) {
            QVERIFY(XCaf::isShapeComponent(componentLabel));
            QVERIFY(XCaf::isShapeReference(componentLabel));
        }

        QCOMPARE(doc->xcaf().topLevelFreeShapes().Size(), 1);
        QCOMPARE(doc->xcaf().topLevelFreeShapes().First(), entityLabel);
    }

    // Clear document: destroy all entities
    while (doc->entityCount() > 0)
        doc->destroyEntity(doc->firstEntityNodeId());

    QCOMPARE(doc->entityCount(), 0);
    QCOMPARE(doc->xcaf().topLevelFreeShapes().Size(), 0);

    // Import file again in document
    {
        const bool okImport = m_ioSystem->importInDocument(doc, docFilePath);
        QVERIFY(okImport);
        QCOMPARE(doc->entityCount(), 1);
        const TDF_Label entityLabel = doc->firstEntityNodeLabel();
        QVERIFY(XCaf::isShapeAssembly(entityLabel));
        QCOMPARE(XCaf::shapeComponents(entityLabel).Size(), 3);
        QCOMPARE(doc->xcaf().topLevelFreeShapes().Size(), 1);
        QCOMPARE(doc->xcaf().topLevelFreeShapes().First(), entityLabel);
    }
}

void TestIO::Regression_bugGitHub166_test()
{
    QFETCH(std::string, strInputFilePath);
    QFETCH(std::string, strOutputFilePath);
    QFETCH(IO::Format, outputFormat);

    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    const bool okImport = m_ioSystem->importInDocument(doc, strInputFilePath);
    QVERIFY(okImport);
    QVERIFY(doc->entityCount() > 0);

    const bool okExport = m_ioSystem->exportItems(
        IO::System::ArgsExport()
            .setTargetFile(strOutputFilePath)
            .setTargetFormat(outputFormat)
            .setItem(ApplicationItem{doc})
        );
    QVERIFY(okExport);
    app->closeDocument(doc);

    doc = app->newDocument();
    const bool okImportOutput = m_ioSystem->importInDocument(doc, strOutputFilePath);
    QVERIFY(okImportOutput);
    QVERIFY(doc->entityCount() > 0);
}

void TestIO::Regression_bugGitHub166_test_data()
{
    QTest::addColumn<std::string>("strInputFilePath");
    QTest::addColumn<std::string>("strOutputFilePath");
    QTest::addColumn<IO::Format>("outputFormat");

    using namespace std::string_literals;
    QTest::newRow("PLY->STL") << "tests/inputs/cube.ply"s << "tests/outputs/cube.stl"s << IO::Format_STL;
    QTest::newRow("STL->PLY") << "tests/inputs/cube.stla"s << "tests/outputs/cube.ply"s << IO::Format_PLY;

    QTest::newRow("OBJ->PLY") << "tests/inputs/cube.obj"s << "tests/outputs/cube.ply"s << IO::Format_PLY;
    QTest::newRow("OBJ->STL") << "tests/inputs/cube.obj"s << "tests/outputs/cube.stl"s << IO::Format_STL;
#ifdef OPENCASCADE_HAVE_RAPIDJSON
    QTest::newRow("glTF->PLY") << "tests/inputs/cube.gltf"s << "tests/outputs/cube.ply"s << IO::Format_PLY;
    QTest::newRow("glTF->STL") << "tests/inputs/cube.gltf"s << "tests/outputs/cube.stl"s << IO::Format_STL;
#endif

#if OCC_VERSION_HEX >= 0x070600
    QTest::newRow("PLY->OBJ") << "tests/inputs/cube.ply"s << "tests/outputs/cube.obj"s << IO::Format_OBJ;
    QTest::newRow("STL->OBJ") << "tests/inputs/cube.stla"s << "tests/outputs/cube.obj"s << IO::Format_OBJ;
#  ifdef OPENCASCADE_HAVE_RAPIDJSON
    QTest::newRow("glTF->OBJ") << "tests/inputs/cube.gltf"s << "tests/outputs/cube.obj"s << IO::Format_OBJ;
    QTest::newRow("OBJ->glTF") << "tests/inputs/cube.obj"s << "tests/outputs/cube.glTF"s << IO::Format_GLTF;
#  endif
#endif
}

void TestIO::Regression_bugGitHub258_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    const bool okImport = m_ioSystem->importInDocument(doc, "tests/inputs/#258_cube.off");
    QVERIFY(okImport);
    QVERIFY(doc->entityCount() == 1);

    const TopoDS_Shape shape = doc->xcaf().shape(doc->firstEntityNodeLabel());
    const TopoDS_Face& face = TopoDS::Face(shape);
    TopLoc_Location locFace;
    auto triangulation = BRep_Tool::Triangulation(face, locFace);
    QVERIFY(!triangulation.IsNull());
    QCOMPARE(triangulation->NbNodes(), 24);
    QCOMPARE(triangulation->NbTriangles(), 12);
}

void TestIO::System_probeFormat_test()
{
    QFETCH(std::string, strFilePath);
    QFETCH(IO::Format, expectedPartFormat);

    QCOMPARE(m_ioSystem->probeFormat(strFilePath), expectedPartFormat);
}

void TestIO::System_probeFormat_test_data()
{
    QTest::addColumn<std::string>("strFilePath");
    QTest::addColumn<IO::Format>("expectedPartFormat");

    using namespace std::string_literals;
    QTest::newRow("cube.step") << "tests/inputs/cube.step"s << IO::Format_STEP;
    QTest::newRow("cube.iges") << "tests/inputs/cube.iges"s << IO::Format_IGES;
    QTest::newRow("cube.brep") << "tests/inputs/cube.brep"s << IO::Format_OCCBREP;
    QTest::newRow("bezier_curve.brep") << "tests/inputs/mayo_bezier_curve.brep"s << IO::Format_OCCBREP;
    QTest::newRow("cube.stla") << "tests/inputs/cube.stla"s << IO::Format_STL;
    QTest::newRow("cube.stlb") << "tests/inputs/cube.stlb"s << IO::Format_STL;
    QTest::newRow("cube.obj") << "tests/inputs/cube.obj"s << IO::Format_OBJ;
    QTest::newRow("cube.ply") << "tests/inputs/cube.ply"s << IO::Format_PLY;
    QTest::newRow("cube.off") << "tests/inputs/cube.off"s << IO::Format_OFF;
    QTest::newRow("cube.wrl") << "tests/inputs/cube.wrl"s << IO::Format_VRML;
}

void TestIO::System_probeFormatDirect_test()
{
    char fileSample[1024];
    IO::System::FormatProbeInput input;

    auto fnSetProbeInput = [&](const FilePath& fp) {
        std::memset(fileSample, 0, std::size(fileSample));
        std::ifstream ifstr;
        ifstr.open(fp, std::ios::in | std::ios::binary);
        ifstr.read(fileSample, std::size(fileSample));

        input.filepath = fp;
        input.contentsBegin = std::string_view(fileSample, ifstr.gcount());
        input.hintFullSize = filepathFileSize(fp);
    };

    fnSetProbeInput("tests/inputs/cube.step");
    QCOMPARE(IO::probeFormat_STEP(input), IO::Format_STEP);

    fnSetProbeInput("tests/inputs/cube.iges");
    QCOMPARE(IO::probeFormat_IGES(input), IO::Format_IGES);

    fnSetProbeInput("tests/inputs/cube.brep");
    QCOMPARE(IO::probeFormat_OCCBREP(input), IO::Format_OCCBREP);

    fnSetProbeInput("tests/inputs/cube.stla");
    QCOMPARE(IO::probeFormat_STL(input), IO::Format_STL);

    fnSetProbeInput("tests/inputs/cube.stlb");
    QCOMPARE(IO::probeFormat_STL(input), IO::Format_STL);

    fnSetProbeInput("tests/inputs/cube.obj");
    QCOMPARE(IO::probeFormat_OBJ(input), IO::Format_OBJ);

    fnSetProbeInput("tests/inputs/cube.ply");
    QCOMPARE(IO::probeFormat_PLY(input), IO::Format_PLY);

    fnSetProbeInput("tests/inputs/cube.off");
    QCOMPARE(IO::probeFormat_OFF(input), IO::Format_OFF);
}

void TestIO::OccStaticVariablesRollback_test()
{
    using Variant = PropertyValueConversion::Variant;
    QFETCH(std::string, varName);
    QFETCH(Variant, varInitValue);
    QFETCH(Variant, varChangeValue);

    auto fnStaticVariableType = [](Variant value) {
        if (std::holds_alternative<int>(value))
            return Interface_ParamInteger;
        else if (std::holds_alternative<double>(value))
            return Interface_ParamReal;
        else if (std::holds_alternative<std::string>(value))
            return Interface_ParamText;
        else
            return Interface_ParamMisc;
    };
    auto fnStaticVariableValue = [](const char* varName, size_t valueTypeIndex) -> Variant {
        if (Variant{0}.index() == valueTypeIndex)
            return Interface_Static::IVal(varName);
        else if (Variant{0.}.index() == valueTypeIndex)
            return Interface_Static::RVal(varName);
        else if (Variant{std::string{}}.index() == valueTypeIndex)
            return std::string{Interface_Static::CVal(varName)};
        else
            return {};
    };

    QCOMPARE(varInitValue.index(), varChangeValue.index());
    const char* cVarName = varName.c_str();
    const std::string strVarInitValue = varInitValue.toString();
    Interface_Static::Init("MAYO", cVarName, fnStaticVariableType(varInitValue), strVarInitValue.c_str());
    QVERIFY(Interface_Static::IsPresent(cVarName));
    QCOMPARE(fnStaticVariableValue(cVarName, varInitValue.index()), varInitValue);

    {
        IO::OccStaticVariablesRollback rollback;
        if (std::holds_alternative<int>(varChangeValue))
            rollback.change(cVarName, varChangeValue.toInt());
        else if (std::holds_alternative<double>(varChangeValue))
            rollback.change(cVarName, varChangeValue.toDouble());
        else if (std::holds_alternative<std::string>(varChangeValue))
            rollback.change(cVarName, varChangeValue.toString());

        QCOMPARE(fnStaticVariableValue(cVarName, varChangeValue.index()), varChangeValue);
    }

    QCOMPARE(fnStaticVariableValue(cVarName, varInitValue.index()), varInitValue);
}

void TestIO::OccStaticVariablesRollback_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<std::string>("varName");
    QTest::addColumn<Variant>("varInitValue");
    QTest::addColumn<Variant>("varChangeValue");

    using namespace std::string_literals;
    QTest::newRow("var_int1") << "mayo.test.variable_int1"s << Variant(25) << Variant(40);
    QTest::newRow("var_int2") << "mayo.test.variable_int2"s << Variant(0) << Variant(5);
    QTest::newRow("var_double1") << "mayo.test.variable_double1"s << Variant(1.5) << Variant(4.5);
    QTest::newRow("var_double2") << "mayo.test.variable_double2"s << Variant(50.7) << Variant(25.8);
    QTest::newRow("var_str1") << "mayo.test.variable_str1"s << Variant("") << Variant("value");
    QTest::newRow("var_str2") << "mayo.test.variable_str2"s << Variant("foo") << Variant("blah");
}

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

void TestIO::initTestCase()
{
    m_ioSystem = new IO::System;

    m_ioSystem->addFactoryReader(std::make_unique<IO::DxfFactoryReader>());
    m_ioSystem->addFactoryReader(std::make_unique<IO::OccFactoryReader>());
    m_ioSystem->addFactoryReader(std::make_unique<IO::OffFactoryReader>());
    m_ioSystem->addFactoryReader(std::make_unique<IO::PlyFactoryReader>());

    m_ioSystem->addFactoryWriter(std::make_unique<IO::OccFactoryWriter>());
    m_ioSystem->addFactoryWriter(std::make_unique<IO::OffFactoryWriter>());
    m_ioSystem->addFactoryWriter(std::make_unique<IO::PlyFactoryWriter>());

    IO::addPredefinedFormatProbes(m_ioSystem);
}

void TestIO::cleanupTestCase()
{
    delete m_ioSystem;
    m_ioSystem = nullptr;
}

} // namespace Mayo
