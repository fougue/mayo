/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test_io.h"

#include "../src/base/application.h"
#include "../src/base/caf_utils.h"
#include "../src/base/io_system.h"
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

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(Mayo::IO::Format)

namespace Mayo {

void TestIO::IO_Reload_bugGitHub332_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    // Import file in document
    const FilePath docFilePath = "tests/inputs/#332_file.stp";
    {
        const bool okImport = m_ioSystem->importInDocument().targetDocument(doc).withFilepath(docFilePath).execute();
        QVERIFY(okImport);
        QCOMPARE(doc->entityCount(), 1);
        const TDF_Label entityLabel = doc->entityLabel(0);
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
        doc->destroyEntity(doc->entityTreeNodeId(0));

    QCOMPARE(doc->entityCount(), 0);
    QCOMPARE(doc->xcaf().topLevelFreeShapes().Size(), 0);

    // Import file again in document
    {
        const bool okImport = m_ioSystem->importInDocument().targetDocument(doc).withFilepath(docFilePath).execute();
        QVERIFY(okImport);
        QCOMPARE(doc->entityCount(), 1);
        const TDF_Label entityLabel = doc->entityLabel(0);
        QVERIFY(XCaf::isShapeAssembly(entityLabel));
        QCOMPARE(XCaf::shapeComponents(entityLabel).Size(), 3);
        QCOMPARE(doc->xcaf().topLevelFreeShapes().Size(), 1);
        QCOMPARE(doc->xcaf().topLevelFreeShapes().First(), entityLabel);
    }
}

void TestIO::IO_probeFormat_test()
{
    QFETCH(QString, strFilePath);
    QFETCH(IO::Format, expectedPartFormat);

    QCOMPARE(m_ioSystem->probeFormat(strFilePath.toStdString()), expectedPartFormat);
}

void TestIO::IO_probeFormat_test_data()
{
    QTest::addColumn<QString>("strFilePath");
    QTest::addColumn<IO::Format>("expectedPartFormat");

    QTest::newRow("cube.step") << "tests/inputs/cube.step" << IO::Format_STEP;
    QTest::newRow("cube.iges") << "tests/inputs/cube.iges" << IO::Format_IGES;
    QTest::newRow("cube.brep") << "tests/inputs/cube.brep" << IO::Format_OCCBREP;
    QTest::newRow("bezier_curve.brep") << "tests/inputs/mayo_bezier_curve.brep" << IO::Format_OCCBREP;
    QTest::newRow("cube.stla") << "tests/inputs/cube.stla" << IO::Format_STL;
    QTest::newRow("cube.stlb") << "tests/inputs/cube.stlb" << IO::Format_STL;
    QTest::newRow("cube.obj") << "tests/inputs/cube.obj" << IO::Format_OBJ;
    QTest::newRow("cube.ply") << "tests/inputs/cube.ply" << IO::Format_PLY;
    QTest::newRow("cube.off") << "tests/inputs/cube.off" << IO::Format_OFF;
    QTest::newRow("cube.wrl") << "tests/inputs/cube.wrl" << IO::Format_VRML;
}

void TestIO::IO_probeFormatDirect_test()
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

void TestIO::IO_OccStaticVariablesRollback_test()
{
    QFETCH(QString, varName);
    QFETCH(QVariant, varInitValue);
    QFETCH(QVariant, varChangeValue);

    auto fnStaticVariableType = [](QVariant value) {
        switch (value.type()) {
        case QVariant::Int: return Interface_ParamInteger;
        case QVariant::Double: return Interface_ParamReal;
        case QVariant::String: return Interface_ParamText;
        default: return Interface_ParamMisc;
        }
    };
    auto fnStaticVariableValue = [](const char* varName, QVariant::Type valueType) -> QVariant {
        switch (valueType) {
        case QVariant::Int: return Interface_Static::IVal(varName);
        case QVariant::Double: return Interface_Static::RVal(varName);
        case QVariant::String: return Interface_Static::CVal(varName);
        default: return {};
        }
    };

    QCOMPARE(varInitValue.type(), varChangeValue.type());
    const QByteArray bytesVarName = varName.toLatin1();
    const char* cVarName = bytesVarName.constData();
    const QByteArray bytesVarInitValue = varInitValue.toString().toLatin1();
    Interface_Static::Init("MAYO", cVarName, fnStaticVariableType(varInitValue), bytesVarInitValue.constData());
    QVERIFY(Interface_Static::IsPresent(cVarName));
    QCOMPARE(fnStaticVariableValue(cVarName, varInitValue.type()), varInitValue);

    {
        IO::OccStaticVariablesRollback rollback;
        if (varChangeValue.type() == QVariant::Int)
            rollback.change(cVarName, varChangeValue.toInt());
        else if (varChangeValue.type() == QVariant::Double)
            rollback.change(cVarName, varChangeValue.toDouble());
        else if (varChangeValue.type() == QVariant::String)
            rollback.change(cVarName, varChangeValue.toString().toStdString());

        QCOMPARE(fnStaticVariableValue(cVarName, varChangeValue.type()), varChangeValue);
    }

    QCOMPARE(fnStaticVariableValue(cVarName, varInitValue.type()), varInitValue);
}

void TestIO::IO_OccStaticVariablesRollback_test_data()
{
    QTest::addColumn<QString>("varName");
    QTest::addColumn<QVariant>("varInitValue");
    QTest::addColumn<QVariant>("varChangeValue");

    QTest::newRow("var_int1") << "mayo.test.variable_int1" << QVariant(25) << QVariant(40);
    QTest::newRow("var_int2") << "mayo.test.variable_int2" << QVariant(0) << QVariant(5);
    QTest::newRow("var_double1") << "mayo.test.variable_double1" << QVariant(1.5) << QVariant(4.5);
    QTest::newRow("var_double2") << "mayo.test.variable_double2" << QVariant(50.7) << QVariant(25.8);
    QTest::newRow("var_str1") << "mayo.test.variable_str1" << QVariant("") << QVariant("value");
    QTest::newRow("var_str2") << "mayo.test.variable_str2" << QVariant("foo") << QVariant("blah");
}

void TestIO::IO_bugGitHub166_test()
{
    QFETCH(QString, strInputFilePath);
    QFETCH(QString, strOutputFilePath);
    QFETCH(IO::Format, outputFormat);

    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    const bool okImport = m_ioSystem->importInDocument()
            .targetDocument(doc)
            .withFilepath(strInputFilePath.toStdString())
            .execute()
        ;
    QVERIFY(okImport);
    QVERIFY(doc->entityCount() > 0);

    const bool okExport = m_ioSystem->exportApplicationItems()
            .targetFile(strOutputFilePath.toStdString())
            .targetFormat(outputFormat)
            .withItem(doc)
            .execute()
        ;
    QVERIFY(okExport);
    app->closeDocument(doc);

    doc = app->newDocument();
    const bool okImportOutput = m_ioSystem->importInDocument()
            .targetDocument(doc)
            .withFilepath(strOutputFilePath.toStdString())
            .execute()
        ;
    QVERIFY(okImportOutput);
    QVERIFY(doc->entityCount() > 0);
}

void TestIO::IO_bugGitHub166_test_data()
{
    QTest::addColumn<QString>("strInputFilePath");
    QTest::addColumn<QString>("strOutputFilePath");
    QTest::addColumn<IO::Format>("outputFormat");

    QTest::newRow("PLY->STL") << "tests/inputs/cube.ply" << "tests/outputs/cube.stl" << IO::Format_STL;
    QTest::newRow("STL->PLY") << "tests/inputs/cube.stla" << "tests/outputs/cube.ply" << IO::Format_PLY;

#if OCC_VERSION_HEX >= 0x070400
    QTest::newRow("OBJ->PLY") << "tests/inputs/cube.obj" << "tests/outputs/cube.ply" << IO::Format_PLY;
    QTest::newRow("OBJ->STL") << "tests/inputs/cube.obj" << "tests/outputs/cube.stl" << IO::Format_STL;
#  ifdef OPENCASCADE_HAVE_RAPIDJSON
    QTest::newRow("glTF->PLY") << "tests/inputs/cube.gltf" << "tests/outputs/cube.ply" << IO::Format_PLY;
    QTest::newRow("glTF->STL") << "tests/inputs/cube.gltf" << "tests/outputs/cube.stl" << IO::Format_STL;
#  endif
#endif

#if OCC_VERSION_HEX >= 0x070600
    QTest::newRow("PLY->OBJ") << "tests/inputs/cube.ply" << "tests/outputs/cube.obj" << IO::Format_OBJ;
    QTest::newRow("STL->OBJ") << "tests/inputs/cube.stla" << "tests/outputs/cube.obj" << IO::Format_OBJ;
#  ifdef OPENCASCADE_HAVE_RAPIDJSON
    QTest::newRow("glTF->OBJ") << "tests/inputs/cube.gltf" << "tests/outputs/cube.obj" << IO::Format_OBJ;
    QTest::newRow("OBJ->glTF") << "tests/inputs/cube.obj" << "tests/outputs/cube.glTF" << IO::Format_GLTF;
#  endif
#endif
}

void TestIO::IO_bugGitHub258_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    const bool okImport = m_ioSystem->importInDocument()
            .targetDocument(doc)
            .withFilepath("tests/inputs/#258_cube.off")
            .execute()
        ;
    QVERIFY(okImport);
    QVERIFY(doc->entityCount() == 1);

    const TopoDS_Shape shape = doc->xcaf().shape(doc->entityLabel(0));
    const TopoDS_Face& face = TopoDS::Face(shape);
    TopLoc_Location locFace;
    auto triangulation = BRep_Tool::Triangulation(face, locFace);
    QVERIFY(!triangulation.IsNull());
    QCOMPARE(triangulation->NbNodes(), 24);
    QCOMPARE(triangulation->NbTriangles(), 12);
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
