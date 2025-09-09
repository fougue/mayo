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
#include "../src/base/libtree.h"
#include "../src/base/occ_handle.h"
#include "../src/base/mesh_utils.h"
#include "../src/base/messenger.h"
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

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <NCollection_String.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TDataStd_Name.hxx>

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
Q_DECLARE_METATYPE(std::vector<gp_Pnt2d>)
Q_DECLARE_METATYPE(Mayo::MeshUtils::Orientation)
Q_DECLARE_METATYPE(Mayo::MessageType)
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
    auto fnAddNewShapeEntity = [](DocumentPtr doc, const char* strEntityName) {
        const TDF_Label shapeLabel = doc->newEntityShapeLabel();
        TDataStd_Name::Set(shapeLabel, to_OccExtString(strEntityName));
        doc->addEntityTreeNode(shapeLabel);
        return shapeLabel;
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
        fnAddNewShapeEntity(doc, "SomeShape");
        QCOMPARE(spyEntityAdded.count, 1);
        QCOMPARE(doc->entityCount(), 1);
        QVERIFY(XCaf::isShape(doc->entityLabel(0)));
        QCOMPARE(CafUtils::labelAttrStdName(doc->entityLabel(0)), to_OccExtString("SomeShape"));

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
        fnAddNewShapeEntity(doc, "Shape1");
        QCOMPARE(doc->entityCount(), 1);

        fnAddNewShapeEntity(doc, "Shape2");
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

void TestBase::MessageCollecter_ignoreSingleMessageType_test()
{
    QFETCH(MessageType, msgTypeToIgnore);

    MessageCollecter msgCollect;
    msgCollect.ignore(msgTypeToIgnore);
    QVERIFY(msgCollect.isIgnored(msgTypeToIgnore));
    for (auto msgType : MetaEnum::values<MessageType>()) {
        if (msgType != msgTypeToIgnore)
            QVERIFY(!msgCollect.isIgnored(msgType));
    }

    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        msgCollect.emitMessage(value, std::string{name} + " message");

    QCOMPARE(msgCollect.messages().size(), MetaEnum::count<MessageType>() - 1);
    for (const auto& msg : msgCollect.messages())
        QVERIFY(msg.type != msgTypeToIgnore);
}

void TestBase::MessageCollecter_ignoreSingleMessageType_test_data()
{
    QTest::addColumn<MessageType>("msgTypeToIgnore");
    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        QTest::newRow(std::string{name}.c_str()) << value;
}

void TestBase::MessageCollecter_only_test()
{
    QFETCH(MessageType, msgTypeSingle);

    MessageCollecter msgCollect;
    msgCollect.only(msgTypeSingle);
    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        msgCollect.emitMessage(value, std::string{name} + " message");

    QCOMPARE(msgCollect.messages().size(), 1);
    QCOMPARE(msgCollect.messages().front().type, msgTypeSingle);
    QCOMPARE(msgCollect.asString(" "), msgCollect.messages().front().text);
    QCOMPARE(msgCollect.asString(" ", msgTypeSingle), msgCollect.messages().front().text);
    QVERIFY(msgCollect.asString(" ").back() != ' ');
}

void TestBase::MessageCollecter_only_test_data()
{
    QTest::addColumn<MessageType>("msgTypeSingle");
    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        QTest::newRow(std::string{name}.c_str()) << value;
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

void TestBase::CafUtils_labelTag_test()
{
    // TODO Add CafUtils::labelTag() test for multi-threaded safety
}

void TestBase::CafUtils_getNamedDataKeys_test()
{
    QVERIFY(CafUtils::getNamedDataKeys({}).empty());
    QCOMPARE(CafUtils::namedDataCount({}), 0);

    auto data = makeOccHandle<TDataStd_NamedData>();
    QVERIFY(CafUtils::getNamedDataKeys(data).empty());
    QCOMPARE(CafUtils::namedDataCount(data), 0);

    data->SetInteger(L"int1", 496);
    data->SetInteger(L"int2", 987);
    data->SetReal(L"double1", 1.214);
    data->SetReal(L"double2", 15.06);
    data->SetByte(L"byte1", 31);
    data->SetString(L"string1", "Mayo");
    data->SetString(L"string2", "Fougue");

    {
        const int cintArray1[] = { 1, 9, 2, 8, 3, 7, 4};
        const TColStd_Array1OfInteger intArray1(cintArray1[0], 1, int(std::size(cintArray1)));
        data->SetArrayOfIntegers(L"intArray1", makeOccHandle<TColStd_HArray1OfInteger>(intArray1));
    }

    {
        const double cdoubleArray1[] = { 1.9, 8.2, 3.7, 6.4, 5.5 };
        const TColStd_Array1OfReal doubleArray1(cdoubleArray1[0], 1, int(std::size(cdoubleArray1)));
        data->SetArrayOfReals(L"doubleArray1", makeOccHandle<TColStd_HArray1OfReal>(doubleArray1));
    }

    auto keys = CafUtils::getNamedDataKeys(data);
    QCOMPARE(CafUtils::namedDataCount(data), keys.size());

    auto fnFindKey = [](const TCollection_ExtendedString& label, const auto& keys) {
        auto it = std::find_if(
            keys.cbegin(), keys.cend(), [&](const auto& dkey) { return dkey.label() == label; }
        );
        return it != keys.cend() ? *it : CafUtils::NamedDataKey{};
    };

    QCOMPARE(fnFindKey(L"int1", keys).type, CafUtils::NamedDataType::Int);
    QCOMPARE(fnFindKey(L"int2", keys).type, CafUtils::NamedDataType::Int);
    QCOMPARE(fnFindKey(L"double1", keys).type, CafUtils::NamedDataType::Double);
    QCOMPARE(fnFindKey(L"double2", keys).type, CafUtils::NamedDataType::Double);
    QCOMPARE(fnFindKey(L"byte1", keys).type, CafUtils::NamedDataType::Byte);
    QCOMPARE(fnFindKey(L"string1", keys).type, CafUtils::NamedDataType::String);
    QCOMPARE(fnFindKey(L"string2", keys).type, CafUtils::NamedDataType::String);
    QCOMPARE(fnFindKey(L"intArray1", keys).type, CafUtils::NamedDataType::IntArray);
    QCOMPARE(fnFindKey(L"doubleArray1", keys).type, CafUtils::NamedDataType::DoubleArray);
    QCOMPARE(fnFindKey(L"?", keys).type, CafUtils::NamedDataType::None);
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

void TestBase::LibTree_nodeRoot_test()
{
    Tree<int> tree;
    tree.appendChild(0, -1);
    tree.appendChild(0, -2);
    QCOMPARE(tree.nodeRoot(0), 0);
}

void TestBase::LibTree_removeRoot_test()
{
    Tree<std::string> tree;
    QCOMPARE(tree.nodeCount(), 0);

    const TreeNodeId root1 = tree.appendChild(0, "root1");
    const TreeNodeId root2 = tree.appendChild(0, "root2");
    QCOMPARE(tree.nodeCount(), 2);
    tree.appendChild(root1, "1-child1");
    tree.appendChild(root1, "1-child2");
    tree.appendChild(root2, "2-child1");
    tree.appendChild(root2, "2-child2");
    tree.appendChild(root2, "2-child3");
    QCOMPARE(tree.nodeCount(), 7);

    tree.removeRoot(root2);
    QVERIFY(tree.nodeCount() > 0);
    tree.removeRoot(root1);
    // All root nodes removed: the count of nodes must be 0
    QCOMPARE(tree.nodeCount(), 0);
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

void TestBase::XCaf_userDefinedAttributes_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    // Must not crash for null labels
    QVERIFY(doc->xcaf().shapeUserDefinedAttributes(TDF_Label{}).IsNull());

    // Must not create user defined attributes if none attached
    const TDF_Label shapeLabel = doc->newEntityShapeLabel();
    TDataStd_Name::Set(shapeLabel, L"Shape1");
    doc->addEntityTreeNode(shapeLabel);
    QVERIFY(doc->xcaf().shapeUserDefinedAttributes(shapeLabel).IsNull());
}

} // namespace Mayo
