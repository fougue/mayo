/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

// Avoid MSVC conflicts with M_E, M_LOG2, ...
#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#  define _USE_MATH_DEFINES
#endif

#include "test_base.h"

#include "../src/base/application.h"
#include "../src/base/brep_utils.h"
#include "../src/base/caf_utils.h"
#include "../src/base/cpp_utils.h"
#include "../src/base/enumeration.h"
#include "../src/base/enumeration_fromenum.h"
#include "../src/base/filepath.h"
#include "../src/base/filepath_conv.h"
#include "../src/base/geom_utils.h"
#include "../src/base/io_system.h"
#include "../src/base/occ_static_variables_rollback.h"
#include "../src/base/libtree.h"
#include "../src/base/occ_handle.h"
#include "../src/base/mesh_utils.h"
#include "../src/base/meta_enum.h"
#include "../src/base/property_builtins.h"
#include "../src/base/property_enumeration.h"
#include "../src/base/property_value_conversion.h"
#include "../src/base/settings.h"
#include "../src/base/string_conv.h"
#include "../src/base/task_manager.h"
#include "../src/base/tkernel_utils.h"
#include "../src/base/unit.h"
#include "../src/base/unit_system.h"
#include "../src/io_dxf/io_dxf.h"
#include "../src/io_occ/io_occ.h"
#include "../src/io_off/io_off_reader.h"
#include "../src/io_off/io_off_writer.h"
#include "../src/io_ply/io_ply_reader.h"
#include "../src/io_ply/io_ply_writer.h"
#include <common/mayo_config.h>

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <Interface_ParamType.hxx>
#include <Interface_Static.hxx>
#include <NCollection_String.hxx>
#include <TopAbs_ShapeEnum.hxx>

#include <QtCore/QtDebug>
#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <gsl/util>
#include <algorithm>
#include <cassert>
#include <clocale>
#include <cmath>
#include <climits>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <unordered_map>

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(Mayo::UnitSystem::TranslateResult)
Q_DECLARE_METATYPE(Mayo::IO::Format)
Q_DECLARE_METATYPE(std::vector<gp_Pnt2d>)
Q_DECLARE_METATYPE(Mayo::MeshUtils::Orientation)
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(Mayo::PropertyValueConversion::Variant)

namespace Mayo {

// For the sake of QCOMPARE()
bool operator==(const UnitSystem::TranslateResult& lhs, const UnitSystem::TranslateResult& rhs)
{
    return std::abs(lhs.value - rhs.value) < 1e-6
           && std::strcmp(lhs.strUnit, rhs.strUnit) == 0
           && std::abs(lhs.factor - rhs.factor) < 1e-6
        ;
}

namespace {

struct frlike_numpunct : public std::numpunct<char> {
    char do_thousands_sep() const override { return ' '; }
    char do_decimal_point() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

std::locale getFrLocale()
{
    auto fnGetLocale = [](const char* name) -> std::optional<std::locale> {
        try {
            return std::locale(name);
        } catch (...) {
            qWarning().noquote() << QString("Locale '%1' not available").arg(name);
        }

        return {};
    };

    // Tests with "fr_FR" locale which is likely to be Windows-1252 or ISO8859-1 on Unix
    std::vector<const char*> frLocaleNames = { "fr_FR.ISO8859-15", "fr_FR.ISO-8859-15" };
#ifndef MAYO_OS_WINDOWS
    // No native utf8 support on Windows(or requires Windows 10 november 2019 update)
    frLocaleNames.push_back("fr_FR.utf8");
#endif
    frLocaleNames.push_back("fr_FR");

    std::optional<std::locale> frLocale;
    for (const char* localeName : frLocaleNames) {
        if (!frLocale)
            frLocale = fnGetLocale(localeName);
    }

    if (!frLocale) {
        frLocale = std::locale(std::cout.getloc(), new frlike_numpunct);
    }
    else {
        const auto& facet = std::use_facet<std::numpunct<char>>(frLocale.value());
        if (facet.decimal_point() != ',' || !std::isspace(facet.thousands_sep(), frLocale.value()))
            frLocale = std::locale(frLocale.value(), new frlike_numpunct);
    }

    assert(frLocale.has_value());
    return frLocale.value();
}

// Equivalent of QSignalSpy for KDBindings signals
struct SignalEmitSpy {
    struct UnknownType {};
    using ArgValue = std::variant<UnknownType, std::int64_t, std::uint64_t>;
    using SignalArguments = std::vector<ArgValue>;

    template<typename... Args>
    SignalEmitSpy(Signal<Args...>* signal) {
        this->sigConnection = signal->connect([=](Args... args) {
            ++this->count;
            SignalArguments sigArgs;
            SignalEmitSpy::recordArgs(&sigArgs, args...);
            this->vecSignals.push_back(std::move(sigArgs));
        });
    }

    ~SignalEmitSpy() {
        this->sigConnection.disconnect();
    }

    static void recordArgs(SignalArguments* /*ptr*/) {
    }

    template<typename Arg, typename... Args>
    static void recordArgs(SignalArguments* ptr, Arg arg, Args... args) {
        if constexpr (std::is_integral_v<Arg>) {
            if constexpr (std::is_signed_v<Arg>) {
                ptr->push_back(static_cast<std::int64_t>(arg));
            }
            else {
                ptr->push_back(static_cast<std::uint64_t>(arg));
            }
        }
        else {
            ptr->push_back(UnknownType{});
        }

        SignalEmitSpy::recordArgs(ptr, args...);
    }

    int count = 0;
    std::vector<SignalArguments> vecSignals;
    SignalConnectionHandle sigConnection;
};

} // namespace

void TestBase::Application_test()
{
    auto app = makeOccHandle<Application>();
    auto fnImportInDocument = [=](const DocumentPtr& doc, const FilePath& fp) {
        return m_ioSystem->importInDocument()
                .targetDocument(doc)
                .withFilepath(fp)
                .execute();
    };
    QCOMPARE(app->documentCount(), 0);

    {   // Add & remove a document
        SignalEmitSpy spyDocAdded(&app->signalDocumentAdded);
        DocumentPtr doc = app->newDocument();
        QVERIFY(!doc.IsNull());
        QCOMPARE(spyDocAdded.count, 1);
        QCOMPARE(app->documentCount(), 1);
        QCOMPARE(app->findIndexOfDocument(doc), 0);
        QCOMPARE(app->findDocumentByIndex(0).get(), doc.get());
        QCOMPARE(app->findDocumentByIdentifier(doc->identifier()).get(), doc.get());

        SignalEmitSpy spyDocClosed(&app->signalDocumentAboutToClose);
        app->closeDocument(doc);
        QCOMPARE(spyDocClosed.count, 1);
        QCOMPARE(app->documentCount(), 0);
    }

    {   // Add & remove an entity
        DocumentPtr doc = app->newDocument();
        auto _ = gsl::finally([=]{ app->closeDocument(doc); });
        QCOMPARE(doc->entityCount(), 0);
        SignalEmitSpy spyEntityAdded(&app->signalDocumentEntityAdded);
        const bool okImport = fnImportInDocument(doc, "tests/inputs/cube.step");
        QVERIFY(okImport);
        QCOMPARE(spyEntityAdded.count, 1);
        QCOMPARE(doc->entityCount(), 1);
        QVERIFY(XCaf::isShape(doc->entityLabel(0)));
        QCOMPARE(CafUtils::labelAttrStdName(doc->entityLabel(0)), to_OccExtString("Cube"));

        SignalEmitSpy spyEntityDestroyed(&app->signalDocumentEntityAboutToBeDestroyed);
        doc->destroyEntity(doc->entityTreeNodeId(0));
        QCOMPARE(spyEntityDestroyed.count, 1);
        QCOMPARE(doc->entityCount(), 0);
    }

    {   // Add mesh entity
        // Add XCAF entity
        // Try to remove mesh and XCAF entities
        // Note: order of entities matters
        QCOMPARE(app->documentCount(), 0);
        DocumentPtr doc = app->newDocument();
        auto _ = gsl::finally([=]{ app->closeDocument(doc); });
        bool okImport = true;
        okImport = fnImportInDocument(doc, "tests/inputs/cube.stlb");
        QVERIFY(okImport);
        QCOMPARE(doc->entityCount(), 1);

        okImport = fnImportInDocument(doc, "tests/inputs/cube.step");
        QVERIFY(okImport);
        QCOMPARE(doc->entityCount(), 2);

        doc->destroyEntity(doc->entityTreeNodeId(0));
        QCOMPARE(doc->entityCount(), 1);
        doc->destroyEntity(doc->entityTreeNodeId(0));
        QCOMPARE(doc->entityCount(), 0);
    }

    QCOMPARE(app->documentCount(), 0);

}

void TestBase::DocumentRefCount_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    QVERIFY(doc->GetRefCount() > 1);
    app->closeDocument(doc);
    QCOMPARE(doc->GetRefCount(), 1);
}

void TestBase::CppUtils_toggle_test()
{
    bool v = false;
    CppUtils::toggle(v);
    QCOMPARE(v, true);
    CppUtils::toggle(v);
    QCOMPARE(v, false);
}

void TestBase::CppUtils_safeStaticCast_test()
{
    QCOMPARE(CppUtils::safeStaticCast<int>(42u), 42);
    QCOMPARE(CppUtils::safeStaticCast<int>(INT_MIN), INT_MIN);
    QCOMPARE(CppUtils::safeStaticCast<int>(INT_MAX), INT_MAX);
    QCOMPARE(CppUtils::safeStaticCast<int>(INT_MAX - 1), INT_MAX - 1);
    QVERIFY_EXCEPTION_THROWN(CppUtils::safeStaticCast<int>(unsigned(INT_MAX) + 1), std::overflow_error);
    QVERIFY_EXCEPTION_THROWN(CppUtils::safeStaticCast<int>(UINT_MAX), std::overflow_error);
}

void TestBase::TextId_test()
{
    struct TextIdContext {
        MAYO_DECLARE_TEXT_ID_FUNCTIONS(TestBase::TextIdContext)
    };

    QVERIFY(TextIdContext::textIdContext() == "TestBase::TextIdContext");
    QVERIFY(TextIdContext::textId("foof").trContext == "TestBase::TextIdContext");
    QVERIFY(TextIdContext::textId("bark").key == "bark");
    QVERIFY(TextIdContext::textIdTr("shktu") == "shktu");
}

void TestBase::FilePath_test()
{
    const char strTestPath[] = "../as1-oc-214 - 測試文件.stp";
    const FilePath testPath = std_filesystem::u8path(strTestPath);

    {
        const TCollection_AsciiString ascStrTestPath(strTestPath);
        QCOMPARE(filepathTo<TCollection_AsciiString>(testPath), ascStrTestPath);
    }

    {
        const TCollection_ExtendedString extStrTestPath(strTestPath, true/*multi-byte*/);
        QCOMPARE(filepathTo<TCollection_ExtendedString>(testPath), extStrTestPath);
    }
}

void TestBase::OccHandle_test()
{
    {
        struct OccHandleTestClass_0 : public Standard_Transient {
            explicit OccHandleTestClass_0() = default;
        };

        auto hnd = makeOccHandle<OccHandleTestClass_0>();
        QCOMPARE(typeid(hnd), typeid(OccHandle<OccHandleTestClass_0>));
        QVERIFY(hnd.get() != nullptr);
    }

    {
        struct OccHandleTestClass_1 : public Standard_Transient {
            explicit OccHandleTestClass_1() = default;
            explicit OccHandleTestClass_1(const std::string& str) : m_str(str) {}
            std::string m_str;
        };

        {
            auto hnd = makeOccHandle<OccHandleTestClass_1>();
            QCOMPARE(typeid(hnd), typeid(OccHandle<OccHandleTestClass_1>));
            QVERIFY(hnd.get() != nullptr);
            QCOMPARE(hnd->m_str, std::string{});
        }

        {
            auto hnd = makeOccHandle<OccHandleTestClass_1>("Test string value");
            QCOMPARE(typeid(hnd), typeid(OccHandle<OccHandleTestClass_1>));
            QVERIFY(hnd.get() != nullptr);
            QCOMPARE(hnd->m_str, "Test string value");
        }
    }
}

void TestBase::PropertyValueConversionVariant_toInt_test()
{
    using Variant = PropertyValueConversion::Variant;
    QFETCH(Variant, variant);
    QFETCH(int, toInt);
    QFETCH(bool, ok);

    bool okActual = false;
    QCOMPARE(variant.toInt(&okActual), toInt);
    QCOMPARE(okActual, ok);
}

void TestBase::PropertyValueConversionVariant_toInt_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<Variant>("variant");
    QTest::addColumn<int>("toInt");
    QTest::addColumn<bool>("ok");

    QTest::newRow("false") << Variant{false} << 0 << false;
    QTest::newRow("true") << Variant{true} << 0 << false;
    QTest::newRow("50.25") << Variant{50.25} << int(std::floor(50.25)) << true;
    QTest::newRow("-50.25") << Variant{-50.25} << int(std::floor(-50.25)) << true;
    QTest::newRow("INT_MAX+1") << Variant{double(INT_MAX) + 1.} << 0 << false;
    QTest::newRow("INT_MIN-1") << Variant{double(INT_MIN) - 1.} << 0 << false;
    QTest::newRow("INT_MAX") << Variant{double(INT_MAX)} << INT_MAX << true;
    QTest::newRow("INT_MIN") << Variant{double(INT_MIN)} << INT_MIN << true;
    QTest::newRow("'58'") << Variant{"58"} << 58 << true;
    QTest::newRow("'4.57'") << Variant{"4.57"} << int(std::floor(4.57)) << true;
    QTest::newRow("'non_int_str'") << Variant{"non_int_str"} << 0 << false;

    const uint8_t bytes[] = { 52, 55 }; // ascii: {'4', '7'}
    QTest::newRow("bytes") << Variant{Span<const uint8_t>(bytes)} << 47 << true;
}

void TestBase::PropertyValueConversionVariant_toString_test()
{
    QFETCH(PropertyValueConversion::Variant, variant);
    QFETCH(std::string, toString);

    bool ok = false;
    if (std::holds_alternative<double>(variant)) {
        const std::string str = variant.toString(&ok);
        QCOMPARE(std::stod(str), std::stod(toString));
    }
    else {
        QCOMPARE(variant.toString(&ok), toString);
    }

    QVERIFY(ok);
}

void TestBase::PropertyValueConversionVariant_toString_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<Variant>("variant");
    QTest::addColumn<std::string>("toString");

    QTest::newRow("false") << Variant{false} << std::string{"false"};
    QTest::newRow("true") << Variant{true} << std::string{"true"};
    QTest::newRow("57") << Variant{57} << std::string{"57"};
    QTest::newRow("4.57f") << Variant{4.57f} << std::string{"4.57"};
    QTest::newRow("1.25") << Variant{1.25} << std::string{"1.25"};
    QTest::newRow("'some string'") << Variant{"some string"} << std::string{"some string"};

    const uint8_t bytes[] = { 48, 65 }; // ascii: {'0', 'A'}
    QTest::newRow("bytes") << Variant{Span<const uint8_t>(bytes)} << std::string{"0A"};
}

void TestBase::PropertyValueConversion_test()
{
    QFETCH(QString, strPropertyName);
    QFETCH(PropertyValueConversion::Variant, variantValue);

    std::unique_ptr<Property> prop;
    if (strPropertyName == PropertyBool::TypeName) {
        prop.reset(new PropertyBool(nullptr, {}));
    }
    else if (strPropertyName == PropertyInt::TypeName) {
        prop.reset(new PropertyInt(nullptr, {}));
    }
    else if (strPropertyName == PropertyDouble::TypeName) {
        prop.reset(new PropertyDouble(nullptr, {}));
    }
    else if (strPropertyName == PropertyString::TypeName) {
        prop.reset(new PropertyString(nullptr, {}));
    }
    else if (strPropertyName == PropertyOccColor::TypeName) {
        prop.reset(new PropertyOccColor(nullptr, {}));
    }
    else if (strPropertyName == PropertyEnumeration::TypeName) {
        enum class MayoTest_Color { Bleu, Blanc, Rouge };
        prop.reset(new PropertyEnum<MayoTest_Color>(nullptr, {}));
    }
    else if (strPropertyName == PropertyFilePath::TypeName) {
        prop.reset(new PropertyFilePath(nullptr, {}));
    }

    QVERIFY(prop);

    PropertyValueConversion conv;
    QVERIFY(conv.fromVariant(prop.get(), variantValue));
    QCOMPARE(conv.toVariant(*prop.get()), variantValue);
}

void TestBase::PropertyValueConversion_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<QString>("strPropertyName");
    QTest::addColumn<Variant>("variantValue");
    QTest::newRow("bool(false)") << PropertyBool::TypeName << Variant(false);
    QTest::newRow("bool(true)") << PropertyBool::TypeName << Variant(true);
    QTest::newRow("int(-50)") << PropertyInt::TypeName << Variant(-50);
    QTest::newRow("int(1979)") << PropertyInt::TypeName << Variant(1979);
    QTest::newRow("double(-1e6)") << PropertyDouble::TypeName << Variant(-1e6);
    QTest::newRow("double(3.1415926535)") << PropertyDouble::TypeName << Variant(3.1415926535);
    QTest::newRow("String(\"test\")") << PropertyString::TypeName << Variant("test");
    QTest::newRow("OccColor(#0000AA)") << PropertyOccColor::TypeName << Variant("#0000AA");
    QTest::newRow("OccColor(#FFFFFF)") << PropertyOccColor::TypeName << Variant("#FFFFFF");
    QTest::newRow("OccColor(#BB0000)") << PropertyOccColor::TypeName << Variant("#BB0000");
    QTest::newRow("Enumeration(Color)") << PropertyEnumeration::TypeName << Variant("Blanc");
}

void TestBase::PropertyValueConversion_bugGitHub219_test()
{
    const std::string strPath = "c:\\é_à_À_œ_ç";
    PropertyValueConversion conv;
    PropertyFilePath propFilePath(nullptr, {});
    const bool ok = conv.fromVariant(&propFilePath, strPath);
    QVERIFY(ok);
    //qDebug() << "strPath:" << QByteArray::fromStdString(strPath);
    //qDebug() << "propFilePath:" << QByteArray::fromStdString(propFilePath.value().u8string());
    QCOMPARE(propFilePath.value().u8string(), strPath);
}

void TestBase::PropertyQuantityValueConversion_test()
{
    QFETCH(QString, strPropertyName);
    QFETCH(PropertyValueConversion::Variant, variantFrom);
    QFETCH(PropertyValueConversion::Variant, variantTo);

    std::unique_ptr<Property> prop;
    if (strPropertyName == "PropertyLength") {
        prop.reset(new PropertyLength(nullptr, {}));
    }
    else if (strPropertyName == "PropertyAngle") {
        prop.reset(new PropertyAngle(nullptr, {}));
    }

    QVERIFY(prop);

    PropertyValueConversion conv;
    conv.setDoubleToStringPrecision(7);
    QVERIFY(conv.fromVariant(prop.get(), variantFrom));
    QCOMPARE(conv.toVariant(*prop.get()), variantTo);
}

void TestBase::PropertyQuantityValueConversion_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<QString>("strPropertyName");
    QTest::addColumn<Variant>("variantFrom");
    QTest::addColumn<Variant>("variantTo");
    QTest::newRow("Length(25mm)") << "PropertyLength" << Variant("25mm") << Variant("25mm");
    QTest::newRow("Length(2m)") << "PropertyLength" << Variant("2m") << Variant("2000mm");
    QTest::newRow("Angle(1.57079rad)") << "PropertyAngle" << Variant("1.57079rad") << Variant("1.57079rad");
    QTest::newRow("Angle(90°)") << "PropertyAngle" << Variant("90°") << Variant("1.570796rad");
}

void TestBase::IO_probeFormat_test()
{
    QFETCH(QString, strFilePath);
    QFETCH(IO::Format, expectedPartFormat);

    QCOMPARE(m_ioSystem->probeFormat(strFilePath.toStdString()), expectedPartFormat);
}

void TestBase::IO_probeFormat_test_data()
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

void TestBase::IO_probeFormatDirect_test()
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

void TestBase::IO_OccStaticVariablesRollback_test()
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

void TestBase::IO_OccStaticVariablesRollback_test_data()
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

void TestBase::IO_bugGitHub166_test()
{
    QFETCH(QString, strInputFilePath);
    QFETCH(QString, strOutputFilePath);
    QFETCH(IO::Format, outputFormat);

    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    const bool okImport = m_ioSystem->importInDocument()
            .targetDocument(doc)
            .withFilepath(strInputFilePath.toStdString())
            .execute();
    QVERIFY(okImport);
    QVERIFY(doc->entityCount() > 0);

    const bool okExport = m_ioSystem->exportApplicationItems()
            .targetFile(strOutputFilePath.toStdString())
            .targetFormat(outputFormat)
            .withItem(doc)
            .execute();
    QVERIFY(okExport);
    app->closeDocument(doc);

    doc = app->newDocument();
    const bool okImportOutput = m_ioSystem->importInDocument()
            .targetDocument(doc)
            .withFilepath(strOutputFilePath.toStdString())
            .execute();
    QVERIFY(okImportOutput);
    QVERIFY(doc->entityCount() > 0);
}

void TestBase::IO_bugGitHub166_test_data()
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

void TestBase::IO_bugGitHub258_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    const bool okImport = m_ioSystem->importInDocument()
                              .targetDocument(doc)
                              .withFilepath("tests/inputs/#258_cube.off")
                              .execute();
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

void TestBase::DoubleToString_test()
{
    const std::locale frLocale = getFrLocale();
    qInfo() << "frLocale:" << frLocale.name().c_str();
    // 1258.
    {
        const std::string str = to_stdString(1258.).locale(frLocale).toUtf8(false);
        QCOMPARE(str.at(0), '1');
        QCOMPARE(str.at(1), std::use_facet<std::numpunct<char>>(frLocale).thousands_sep());
        QCOMPARE(str.substr(2, 3), "258");
    }

    // 57.89
    {
        QCOMPARE(to_stdString(57.89).locale(frLocale).get(), "57,89");
    }

    // Tests with "C" locale
    const std::locale& cLocale = std::locale::classic();
    QCOMPARE(to_stdString(0.5578).locale(cLocale).decimalCount(4).get(), "0.5578");
    QCOMPARE(to_stdString(0.5578).locale(cLocale).decimalCount(6).get(), "0.5578");
    QCOMPARE(to_stdString(0.5578).locale(cLocale).decimalCount(6).removeTrailingZeroes(false).get(), "0.557800");
    QCOMPARE(to_stdString(0.0).locale(cLocale).decimalCount(6).get(), "0");
    QCOMPARE(to_stdString(-45.6789).locale(cLocale).decimalCount(6).get(), "-45.6789");
}

void TestBase::StringConv_test()
{
    const std::string stdStr = to_stdString(14758.5).locale(getFrLocale());
    const auto occExtStr = to_OccExtString(stdStr);
    QCOMPARE(stdStr, to_stdString(occExtStr));
}

void TestBase::BRepUtils_test()
{
    QVERIFY(BRepUtils::moreComplex(TopAbs_COMPOUND, TopAbs_SOLID));
    QVERIFY(BRepUtils::moreComplex(TopAbs_SOLID, TopAbs_SHELL));
    QVERIFY(BRepUtils::moreComplex(TopAbs_SHELL, TopAbs_FACE));
    QVERIFY(BRepUtils::moreComplex(TopAbs_FACE, TopAbs_EDGE));
    QVERIFY(BRepUtils::moreComplex(TopAbs_EDGE, TopAbs_VERTEX));

    {
        const TopoDS_Shape shapeNull;
        const TopoDS_Shape shapeBase = BRepPrimAPI_MakeBox(25, 25, 25);
        const TopoDS_Shape shapeCopy = shapeBase;
        const TopoDS_Shape shapeOther = BRepPrimAPI_MakeBox(40, 40, 40);
        QCOMPARE(BRepUtils::hashCode(shapeNull), BRepUtils::hashCode(TopoDS_Shape{}));
        QCOMPARE(BRepUtils::hashCode(shapeBase), BRepUtils::hashCode(shapeCopy));
        QVERIFY(BRepUtils::hashCode(shapeBase) != BRepUtils::hashCode(shapeOther));
    }
}

void TestBase::CafUtils_test()
{
    // TODO Add CafUtils::labelTag() test for multi-threaded safety
}

void TestBase::MeshUtils_orientation_test()
{
    struct BasicPolyline2d : public Mayo::MeshUtils::AdaptorPolyline2d {
        gp_Pnt2d pointAt(int index) const override { return this->vecPoint.at(index); }
        int pointCount() const override { return int(this->vecPoint.size()); }
        std::vector<gp_Pnt2d> vecPoint;
    };

    QFETCH(std::vector<gp_Pnt2d>, vecPoint);
    QFETCH(Mayo::MeshUtils::Orientation, orientation);
    BasicPolyline2d polyline2d;
    polyline2d.vecPoint = std::move(vecPoint);
    QCOMPARE(Mayo::MeshUtils::orientation(polyline2d), orientation);
}

void TestBase::MeshUtils_orientation_test_data()
{
    QTest::addColumn<std::vector<gp_Pnt2d>>("vecPoint");
    QTest::addColumn<Mayo::MeshUtils::Orientation>("orientation");

    std::vector<gp_Pnt2d> vecPoint;
    vecPoint.push_back(gp_Pnt2d(0, 0));
    vecPoint.push_back(gp_Pnt2d(0, 10));
    vecPoint.push_back(gp_Pnt2d(10, 10));
    vecPoint.push_back(gp_Pnt2d(10, 0));
    vecPoint.push_back(gp_Pnt2d(0, 0)); // Closed polyline
    QTest::newRow("case1") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    vecPoint.erase(std::prev(vecPoint.end())); // Open polyline
    QTest::newRow("case2") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    std::reverse(vecPoint.begin(), vecPoint.end());
    QTest::newRow("case3") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

    vecPoint.clear();
    vecPoint.push_back(gp_Pnt2d(0, 0));
    vecPoint.push_back(gp_Pnt2d(0, 10));
    vecPoint.push_back(gp_Pnt2d(10, 10));
    QTest::newRow("case4") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    vecPoint.clear();
    vecPoint.push_back(gp_Pnt2d(0, 0));
    vecPoint.push_back(gp_Pnt2d(0, 10));
    vecPoint.push_back(gp_Pnt2d(-10, 10));
    vecPoint.push_back(gp_Pnt2d(-10, 0));
    QTest::newRow("case5") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

    std::reverse(vecPoint.begin(), vecPoint.end());
    QTest::newRow("case6") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;

    {
        QFile file("tests/inputs/mayo_bezier_curve.brep");
        QVERIFY(file.open(QIODevice::ReadOnly));

        const TopoDS_Shape shape = BRepUtils::shapeFromString(file.readAll().toStdString());
        QVERIFY(!shape.IsNull());

        TopoDS_Edge edge;
        BRepUtils::forEachSubShape(shape, TopAbs_EDGE, [&](const TopoDS_Shape& subShape) {
            edge = TopoDS::Edge(subShape);
        });
        QVERIFY(!edge.IsNull());

        const GCPnts_TangentialDeflection discr(BRepAdaptor_Curve(edge), 0.1, 0.1);
        vecPoint.clear();
        for (int i = 1; i <= discr.NbPoints(); ++i) {
            const gp_Pnt pnt = discr.Value(i);
            vecPoint.push_back(gp_Pnt2d(pnt.X(), pnt.Y()));
        }

        QTest::newRow("case7") << vecPoint << Mayo::MeshUtils::Orientation::CounterClockwise;

        std::reverse(vecPoint.begin(), vecPoint.end());
        QTest::newRow("case8") << vecPoint << Mayo::MeshUtils::Orientation::Clockwise;
    }
}

void TestBase::Enumeration_test()
{
    enum class TestBase_Enum1 { Value0, Value1, Value2, Value3, Value4 };
    using TestEnumType = TestBase_Enum1;

    Enumeration baseEnum = Enumeration::fromType<TestEnumType>();
    QVERIFY(!baseEnum.empty());
    QCOMPARE(baseEnum.size(), MetaEnum::count<TestEnumType>());
    QCOMPARE(baseEnum.items().size(), static_cast<unsigned>(baseEnum.size()));
    for (const auto& enumEntry : MetaEnum::entries<TestEnumType>()) {
        QVERIFY(baseEnum.contains(enumEntry.second));
        QCOMPARE(baseEnum.findValueByName(enumEntry.second), int(enumEntry.first));
        QCOMPARE(baseEnum.findItemByName(enumEntry.second)->value, int(enumEntry.first));
        QCOMPARE(baseEnum.findItemByName(enumEntry.second)->name.key, enumEntry.second);
        QCOMPARE(baseEnum.findItemByValue(enumEntry.first), baseEnum.findItemByName(enumEntry.second));
        QCOMPARE(baseEnum.findNameByValue(enumEntry.first), enumEntry.second);
        QCOMPARE(baseEnum.itemAt(baseEnum.findIndexByValue(enumEntry.first)).value, int(enumEntry.first));
        QCOMPARE(baseEnum.itemAt(baseEnum.findIndexByValue(enumEntry.first)).name.key, enumEntry.second);
    }

    baseEnum.chopPrefix("Value");
    for (const Enumeration::Item& item : baseEnum.items()) {
        const int index = std::atoi(item.name.key.data());
        QCOMPARE(&baseEnum.itemAt(index), &item);
    }

    baseEnum.changeTrContext("newTrContext");
    for (const Enumeration::Item& item : baseEnum.items()) {
        QCOMPARE(item.name.trContext, "newTrContext");
    }
}

void TestBase::MetaEnum_test()
{
    QCOMPARE(MetaEnum::name(TopAbs_VERTEX), "TopAbs_VERTEX");
    QCOMPARE(MetaEnum::name(TopAbs_EDGE), "TopAbs_EDGE");
    QCOMPARE(MetaEnum::name(TopAbs_WIRE), "TopAbs_WIRE");
    QCOMPARE(MetaEnum::name(TopAbs_FACE), "TopAbs_FACE");
    QCOMPARE(MetaEnum::name(TopAbs_SHELL), "TopAbs_SHELL");
    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_SOLID, "TopAbs_"), "SOLID");
    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_COMPOUND, "TopAbs_"), "COMPOUND");

    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_VERTEX, "Abs"), "TopAbs_VERTEX");
    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_VERTEX, ""), "TopAbs_VERTEX");
}

void TestBase::MeshUtils_test()
{
    // Create box
    QFETCH(double, boxDx);
    QFETCH(double, boxDy);
    QFETCH(double, boxDz);
    const TopoDS_Shape shapeBox = BRepPrimAPI_MakeBox(boxDx, boxDy, boxDz);

    // Mesh box
    {
        BRepMesh_IncrementalMesh mesher(shapeBox, 0.1);
        mesher.Perform();
        QVERIFY(mesher.IsDone());
    }

    // Count nodes and triangles
    int countNode = 0;
    int countTriangle = 0;
    BRepUtils::forEachSubFace(shapeBox, [&](const TopoDS_Face& face) {
        TopLoc_Location loc;
        const OccHandle<Poly_Triangulation>& polyTri = BRep_Tool::Triangulation(face, loc);
        if (!polyTri.IsNull()) {
            countNode += polyTri->NbNodes();
            countTriangle += polyTri->NbTriangles();
        }
    });

    // Merge all face triangulations into one
    auto polyTriBox = makeOccHandle<Poly_Triangulation>(countNode, countTriangle, false);
    {
        int idNodeOffset = 0;
        int idTriangleOffset = 0;
        BRepUtils::forEachSubFace(shapeBox, [&](const TopoDS_Face& face) {
            TopLoc_Location loc;
            const OccHandle<Poly_Triangulation>& polyTri = BRep_Tool::Triangulation(face, loc);
            if (!polyTri.IsNull()) {
                for (int i = 1; i <= polyTri->NbNodes(); ++i)
                    MeshUtils::setNode(polyTriBox, idNodeOffset + i, polyTri->Node(i));

                for (int i = 1; i <= polyTri->NbTriangles(); ++i) {
                    int n1, n2, n3;
                    polyTri->Triangle(i).Get(n1, n2, n3);
                    MeshUtils::setTriangle(
                        polyTriBox,
                        idTriangleOffset + i,
                        { idNodeOffset + n1, idNodeOffset + n2, idNodeOffset + n3 }
                    );
                }

                idNodeOffset += polyTri->NbNodes();
                idTriangleOffset += polyTri->NbTriangles();
            }
        });
    }

    // Checks
    QCOMPARE(MeshUtils::triangulationVolume(polyTriBox),
             double(boxDx * boxDy * boxDz));
    QCOMPARE(MeshUtils::triangulationArea(polyTriBox),
             double(2 * boxDx * boxDy + 2 * boxDy * boxDz + 2 * boxDx * boxDz));
}

void TestBase::MeshUtils_test_data()
{
    QTest::addColumn<double>("boxDx");
    QTest::addColumn<double>("boxDy");
    QTest::addColumn<double>("boxDz");

    QTest::newRow("case1") << 10. << 15. << 20.;
    QTest::newRow("case2") << 0.1 << 0.25 << 0.044;
    QTest::newRow("case3") << 1e5 << 1e6 << 1e7;
    QTest::newRow("case4") << 40. << 50. << 70.;
}

void TestBase::Quantity_test()
{
    const QuantityArea area = (10 * Quantity_Millimeter) * (5 * Quantity_Centimeter);
    QCOMPARE(area.value(), 500.);
    QCOMPARE((Quantity_Millimeter / 5.).value(), 1/5.);
}

void TestBase::TKernelUtils_colorToHex_test()
{
    QFETCH(int, red);
    QFETCH(int, green);
    QFETCH(int, blue);
    QFETCH(QString, strHexColor);

    const Quantity_Color color(red / 255., green / 255., blue / 255., Quantity_TOC_RGB);
    const std::string strHexColorActual = TKernelUtils::colorToHex(color);
    QCOMPARE(QString::fromStdString(strHexColorActual), strHexColor);
}

void TestBase::TKernelUtils_colorToHex_test_data()
{
    QTest::addColumn<int>("red");
    QTest::addColumn<int>("green");
    QTest::addColumn<int>("blue");
    QTest::addColumn<QString>("strHexColor");

    QTest::newRow("RGB(  0,  0,  0)") << 0 << 0 << 0 << "#000000";
    QTest::newRow("RGB(255,255,255)") << 255 << 255 << 255 << "#FFFFFF";
    QTest::newRow("RGB(  5,  5,  5)") << 5 << 5 << 5 << "#050505";
    QTest::newRow("RGB(155,208, 67)") << 155 << 208 << 67 << "#9BD043";
    QTest::newRow("RGB(100,150,200)") << 100 << 150 << 200 << "#6496C8";
}

void TestBase::TKernelUtils_colorFromHex_test()
{
    QFETCH(int, red);
    QFETCH(int, green);
    QFETCH(int, blue);
    QFETCH(QString, strHexColor);
    const Quantity_Color expectedColor(red / 255., green / 255., blue / 255., Quantity_TOC_RGB);

    Quantity_Color actualColor;
    QVERIFY(TKernelUtils::colorFromHex(strHexColor.toStdString(), &actualColor));
    QCOMPARE(actualColor, expectedColor);
}

void TestBase::TKernelUtils_colorFromHex_test_data()
{
    QTest::addColumn<int>("red");
    QTest::addColumn<int>("green");
    QTest::addColumn<int>("blue");
    QTest::addColumn<QString>("strHexColor");

    QTest::newRow("RGB(  0,  0,  0)") << 0 << 0 << 0 << "#000000";
    QTest::newRow("RGB(255,255,255)") << 255 << 255 << 255 << "#FFFFFF";
    QTest::newRow("RGB(  5,  5,  5)") << 5 << 5 << 5 << "#050505";
    QTest::newRow("RGB(155,208, 67)") << 155 << 208 << 67 << "#9BD043";
    QTest::newRow("RGB(100,150,200)") << 100 << 150 << 200 << "#6496C8";
}

namespace {

class TestProperties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::TestProperties)
public:
    TestProperties(Settings* settings)
        : PropertyGroup(settings),
          groupId_main(settings->addGroup(textId("main")))
    {
        settings->addSetting(&this->someInt, groupId_main);
        settings->addResetFunction(groupId_main, [&]{
            this->someInt.setValue(-1);
        });
    }

    const Settings::GroupIndex groupId_main;
    PropertyInt someInt{ this, textId("someInt") };
};

class TestSettingsStorage : public Settings::Storage {
public:
    bool contains(std::string_view key) const override
    {
        return m_mapValue.find(key) != m_mapValue.cend();
    }

    Settings::Variant value(std::string_view key) const override
    {
        auto it = m_mapValue.find(key);
        return it != m_mapValue.cend() ? it->second : Settings::Variant{};
    }

    void setValue(std::string_view key, const Settings::Variant& value) override
    {
        m_mapValue.insert_or_assign(key, value);
    }

    void sync() override
    {
    }

private:
    std::unordered_map<std::string_view, Settings::Variant> m_mapValue;
};

} // namespace

void TestBase::Settings_test()
{
    Settings settings;
    {
        auto settingsStorage = std::make_unique<TestSettingsStorage>();
        settingsStorage->setValue("main/someInt", Settings::Variant{5});

        const uint8_t bytes[] = { 97, 98, 99, 100, 101, 95, 49, 50, 51, 52, 53 };
        const Settings::Variant bytesVar(Span<const uint8_t>(bytes, std::size(bytes)));
        QVERIFY(std::holds_alternative<std::vector<uint8_t>>(bytesVar));
        settingsStorage->setValue("main/someTestData", bytesVar);

        settings.setStorage(std::move(settingsStorage));
    }

    TestProperties props(&settings);

    settings.resetAll();
    QCOMPARE(props.someInt.value(), -1);

    settings.load();
    QCOMPARE(props.someInt.value(), 5);
}

void TestBase::UnitSystem_test()
{
    QFETCH(UnitSystem::TranslateResult, trResultActual);
    QFETCH(UnitSystem::TranslateResult, trResultExpected);
    QCOMPARE(trResultActual, trResultExpected);
}

void TestBase::UnitSystem_test_data()
{
    QTest::addColumn<UnitSystem::TranslateResult>("trResultActual");
    QTest::addColumn<UnitSystem::TranslateResult>("trResultExpected");

    const UnitSystem::Schema schemaSI = UnitSystem::SI;
    QTest::newRow("80mm")
            << UnitSystem::translate(schemaSI, 80 * Quantity_Millimeter)
            << UnitSystem::TranslateResult{ 80., "mm", 1. };
    QTest::newRow("8cm")
            << UnitSystem::translate(schemaSI, 8 * Quantity_Centimeter)
            << UnitSystem::TranslateResult{ 80., "mm", 1. };
    QTest::newRow("8m")
            << UnitSystem::translate(schemaSI, 8 * Quantity_Meter)
            << UnitSystem::TranslateResult{ 8000., "mm", 1. };
    QTest::newRow("50mm²")
            << UnitSystem::translate(schemaSI, 0.5 * Quantity_SquareCentimeter)
            << UnitSystem::TranslateResult{ 50., "mm²", 1. };
    QTest::newRow("50kg/m³")
            << UnitSystem::translate(schemaSI, 25 * Quantity_KilogramPerCubicMeter)
            << UnitSystem::TranslateResult{ 25., "kg/m³", 1. };
    QTest::newRow("40kg/m³")
            << UnitSystem::translate(schemaSI, 0.04 * Quantity_GramPerCubicCentimeter)
            << UnitSystem::TranslateResult{ 40., "kg/m³", 1. };

    constexpr double radDeg = Quantity_Degree.value();
    QTest::newRow("degrees(PIrad)")
            << UnitSystem::degrees(3.14159265358979323846 * Quantity_Radian)
            << UnitSystem::TranslateResult{ 180., "°", radDeg };

    QTest::newRow("time(1s)")
            << UnitSystem::milliseconds(1 * Quantity_Second)
            << UnitSystem::TranslateResult{ 1000., "ms", Quantity_Millisecond.value() };
    QTest::newRow("time(5s)")
            << UnitSystem::milliseconds(5 * Quantity_Second)
            << UnitSystem::TranslateResult{ 5000., "ms", Quantity_Millisecond.value() };
    QTest::newRow("time(2min)")
            << UnitSystem::milliseconds(2 * Quantity_Minute)
            << UnitSystem::TranslateResult{ 2 * 60 * 1000., "ms", Quantity_Millisecond.value() };
}

void TestBase::LibTask_test()
{
    struct ProgressRecord {
        TaskId taskId;
        int value;
    };

    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([=](TaskProgress* progress) {
        {
            TaskProgress subProgress(progress, 40);
            for (int i = 0; i <= 100; ++i)
                subProgress.setValue(i);
        }

        {
            TaskProgress subProgress(progress, 60);
            for (int i = 0; i <= 100; ++i)
                subProgress.setValue(i);
        }
    });
    std::vector<ProgressRecord> vecProgressRec;
    taskMgr.signalProgressChanged.connectSlot([&](TaskId taskId, int pct) {
        vecProgressRec.push_back({ taskId, pct });
    });

    SignalEmitSpy sigStarted(&taskMgr.signalStarted);
    SignalEmitSpy sigEnded(&taskMgr.signalEnded);
    taskMgr.run(taskId);
    taskMgr.waitForDone(taskId);

    QCOMPARE(sigStarted.count, 1);
    QCOMPARE(sigEnded.count, 1);
    QCOMPARE(std::get<TaskId>(sigStarted.vecSignals.front().at(0)), taskId);
    QCOMPARE(std::get<TaskId>(sigEnded.vecSignals.front().at(0)), taskId);
    QVERIFY(!vecProgressRec.empty());
    int prevPct = 0;
    for (const ProgressRecord& rec : vecProgressRec) {
        QCOMPARE(rec.taskId, taskId);
        QVERIFY(prevPct <= rec.value);
        prevPct = rec.value;
    }

    QCOMPARE(vecProgressRec.front().value, 0);
    QCOMPARE(vecProgressRec.back().value, 100);
}

void TestBase::LibTree_test()
{
    const TreeNodeId nullptrId = 0;
    Tree<std::string> tree;
    std::vector<TreeNodeId> vecTreeNodeId;
    auto fnAppendChild = [&](TreeNodeId parentId, const char* str) {
        const TreeNodeId id = tree.appendChild(parentId, str);
        vecTreeNodeId.push_back(id);
        return id;
    };
    const TreeNodeId n0 = fnAppendChild(nullptrId, "0");
    const TreeNodeId n0_1 = fnAppendChild(n0, "0-1");
    const TreeNodeId n0_2 = fnAppendChild(n0, "0-2");
    const TreeNodeId n0_1_1 = fnAppendChild(n0_1, "0-1-1");
    const TreeNodeId n0_1_2 = fnAppendChild(n0_1, "0-1-2");
    std::sort(vecTreeNodeId.begin(), vecTreeNodeId.end());

    QCOMPARE(tree.nodeParent(n0_1), n0);
    QCOMPARE(tree.nodeParent(n0_2), n0);
    QCOMPARE(tree.nodeParent(n0_1_1), n0_1);
    QCOMPARE(tree.nodeParent(n0_1_2), n0_1);
    QCOMPARE(tree.nodeChildFirst(n0_1), n0_1_1);
    QCOMPARE(tree.nodeChildLast(n0_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingNext(n0_1_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingPrevious(n0_1_2), n0_1_1);
    QCOMPARE(tree.nodeSiblingNext(n0_1_2), nullptrId);

    {
        std::string strPreOrder;
        traverseTree_preOrder(tree, [&](TreeNodeId id) {
            strPreOrder += " " + tree.nodeData(id);
        });
        QCOMPARE(strPreOrder, " 0 0-1 0-1-1 0-1-2 0-2");
    }

    {
        std::string strPostOrder;
        traverseTree_postOrder(tree, [&](TreeNodeId id) {
            strPostOrder += " " + tree.nodeData(id);
        });
        QCOMPARE(strPostOrder, " 0-1-1 0-1-2 0-1 0-2 0");
    }

    {
        std::vector<TreeNodeId> vecTreeNodeIdVisited;
        traverseTree_unorder(tree, [&](TreeNodeId id) {
            vecTreeNodeIdVisited.push_back(id);
        });
        std::sort(vecTreeNodeIdVisited.begin(), vecTreeNodeIdVisited.end());
        QCOMPARE(vecTreeNodeIdVisited, vecTreeNodeId);
    }
}

void TestBase::Span_test()
{
    const std::vector<std::string> vecString = { "first", "second", "third", "fourth", "fifth" };
    const std::string& item0 = vecString.at(0);
    const std::string& item1 = vecString.at(1);
    const std::string& item2 = vecString.at(2);
    const std::string& item3 = vecString.at(3);
    const std::string& item4 = vecString.at(4);
    QCOMPARE(Span_itemIndex(vecString, item0), 0);
    QCOMPARE(Span_itemIndex(vecString, item1), 1);
    QCOMPARE(Span_itemIndex(vecString, item2), 2);
    QCOMPARE(Span_itemIndex(vecString, item3), 3);
    QCOMPARE(Span_itemIndex(vecString, item4), 4);
}

void TestBase::initTestCase()
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

void TestBase::cleanupTestCase()
{
    delete m_ioSystem;
    m_ioSystem = nullptr;
}

} // namespace Mayo

