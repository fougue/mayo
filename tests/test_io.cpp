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
