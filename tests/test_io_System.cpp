/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_io.h"

#include "../src/base/application.h"
#include "../src/base/message_collecter.h"
#include "../src/base/io_system.h"
#include "../src/base/meta_enum.h"
#include "signal_emit_spy.h"

#include <QtTest/QtTest>

#include <string>

namespace Mayo { namespace {
enum class ThrowLocation;
enum class ExceptionType;
}}

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(Mayo::IO::Format)
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(Mayo::ThrowLocation)
Q_DECLARE_METATYPE(Mayo::ExceptionType)

namespace Mayo {

namespace {

struct OtherException {};

enum class ThrowLocation {
    Reader_readFile, Reader_transfer, Writer_writeFile, Writer_transfer
};

enum class ExceptionType {
    Std, Occt, Other
};

const char* exceptionMsg(ThrowLocation throwLocation)
{
    switch (throwLocation) {
    case ThrowLocation::Reader_transfer: return "Exception in Reader::transfer()";
    case ThrowLocation::Reader_readFile: return "Exception in Reader::readFile()";
    case ThrowLocation::Writer_transfer: return "Exception in Writer::transfer()";
    case ThrowLocation::Writer_writeFile: return "Exception in Writer::writeFile()";
    }
    return "";
}

class ThrowingBase {
public:
    ThrowingBase(ThrowLocation throwLocation, ExceptionType exceptionType)
        : m_throwLocation(throwLocation), m_exceptionType(exceptionType)
    { }

protected:
    template<typename ReturnValueType>
    ReturnValueType throwExceptionIfLocation(ThrowLocation location, ReturnValueType retValue) const
    {
        if (location == m_throwLocation) {
            switch (m_exceptionType) {
            case ExceptionType::Std:   throw std::runtime_error(exceptionMsg(m_throwLocation));
            case ExceptionType::Occt:  throw Standard_Failure(exceptionMsg(m_throwLocation));
            case ExceptionType::Other: throw OtherException{};
            }
        }

        return retValue;
    }

private:
    ThrowLocation m_throwLocation;
    ExceptionType m_exceptionType;
};

template<typename ThrowingClassType>
class ThrowingFactoryBase {
public:
    ThrowingFactoryBase(IO::Format format, ThrowLocation throwLocation, ExceptionType exceptionType)
        : m_format(format), m_throwLocation(throwLocation), m_exceptionType(exceptionType)
    { }

protected:
    gsl::span<const IO::Format> base_formats() const {
        return gsl::span<const IO::Format>(&m_format, 1);
    }

    std::unique_ptr<ThrowingClassType> base_create(IO::Format format) const {
        if (format == m_format)
            return std::make_unique<ThrowingClassType>(m_throwLocation, m_exceptionType);
        return {};
    }

private:
    IO::Format m_format;
    ThrowLocation m_throwLocation;
    ExceptionType m_exceptionType;
};

class ThrowingReader : public ThrowingBase, public IO::Reader {
public:
    using ThrowingBase::ThrowingBase;

    bool readFile(const FilePath&, TaskProgress*) override {
        return throwExceptionIfLocation(ThrowLocation::Reader_readFile, true);
    }

    NCollection_Sequence<TDF_Label> transfer(DocumentPtr, TaskProgress*) override {
        return throwExceptionIfLocation(ThrowLocation::Reader_transfer, NCollection_Sequence<TDF_Label>{});
    }

    void applyProperties(const PropertyGroup*) override {
    }
};

class ThrowingWriter : public ThrowingBase, public IO::Writer {
public:
    using ThrowingBase::ThrowingBase;

    bool transfer(gsl::span<const ApplicationItem>, TaskProgress*) override {
        return throwExceptionIfLocation(ThrowLocation::Writer_transfer, true);
    }

    bool writeFile(const FilePath&, TaskProgress*) override {
        return throwExceptionIfLocation(ThrowLocation::Writer_writeFile, true);
    }

    void applyProperties(const PropertyGroup*) override {
    }
};

class ThrowingFactoryReader : public ThrowingFactoryBase<ThrowingReader>, public IO::FactoryReader {
public:
    using ThrowingFactoryBase<ThrowingReader>::ThrowingFactoryBase;
    gsl::span<const IO::Format> formats() const override { return base_formats(); }
    std::unique_ptr<IO::Reader> create(IO::Format format) const override { return base_create(format); }
    std::unique_ptr<PropertyGroup> createProperties(IO::Format, PropertyGroup*) const override { return {}; }
};

class ThrowingFactoryWriter : public ThrowingFactoryBase<ThrowingWriter>, public IO::FactoryWriter {
public:
    using ThrowingFactoryBase<ThrowingWriter>::ThrowingFactoryBase;
    gsl::span<const IO::Format> formats() const override { return base_formats(); }
    std::unique_ptr<IO::Writer> create(IO::Format format) const override { return base_create(format); }
    std::unique_ptr<PropertyGroup> createProperties(IO::Format, PropertyGroup*) const override {return {}; }
};

const std::string& staticStringStore(std::string_view str)
{
    static std::vector<std::string> strStore;
    strStore.push_back(std::string{str});
    return strStore.back();
}

} // namespace

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

void TestIO::System_importInDocument_catchVrmlReaderSendFail_test()
{
#if OCC_VERSION_HEX >= 0x070700
    // VRML reader available starting from Opencascade >= 7.7.0
    // Relates to GitHub #268 and #269
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();
    MessageCollecter msgCollect;
    msgCollect.only(MessageType::Error);
    const bool okImport = m_ioSystem->importInDocument(
        IO::System::ArgsImport()
            .setTargetDocument(doc)
            .setFilepath("tests/inputs/#268_ambient-intensity-outofrange.wrl")
            .setMessenger(&msgCollect)
        );
    QVERIFY(!okImport);
    QCOMPARE(msgCollect.messageCount(MessageType::Error), 1);
    // Should be message "Error in VrmlAPI_CafReader: IrrelevantNumberoccurred at line (...)"
    QVERIFY(msgCollect.messages()[0].text.find("VrmlAPI_CafReader") != std::string::npos);
#endif
}

void TestIO::System_importInDocumentReaderException_test()
{
    QFETCH(ThrowLocation, throwLocation);
    QFETCH(ExceptionType, exceptionType);

    IO::System ioSystem;
    IO::addPredefinedFormatProbes(&ioSystem);
    ioSystem.addFactoryReader(
        std::make_unique<ThrowingFactoryReader>(IO::Format::Format_OCCBREP, throwLocation, exceptionType)
    );

    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    MessageCollecter msgCollect;
    msgCollect.only(MessageType::Error);

    bool importOk = false;
    try {
        importOk = ioSystem.importInDocument(
            IO::System::ArgsImport()
            .setTargetDocument(doc).setFilepath("test.brep").setMessenger(&msgCollect)
        );
    }
    catch (...) {
        QFAIL("IO::System::importInDocument() must not throw");
    }

    QVERIFY(!importOk);
    QCOMPARE(msgCollect.messageCount(MessageType::Error), 1);
    if (exceptionType != ExceptionType::Other)
        QVERIFY(msgCollect.messages()[0].text.find(exceptionMsg(throwLocation)) != std::string::npos);
}

void TestIO::System_importInDocumentReaderException_test_data()
{
    QTest::addColumn<ThrowLocation>("throwLocation");
    QTest::addColumn<ExceptionType>("exceptionType");

    auto c_str = [](std::string_view lhs, std::string_view rhs) {
        return staticStringStore(std::string{lhs}.append(rhs)).c_str();
    };

    for (const auto& [value, name] : MetaEnum::entries<ExceptionType>())
        QTest::newRow(c_str("readFile_Exception", name)) << ThrowLocation::Reader_readFile << value;

    for (const auto& [value, name] : MetaEnum::entries<ExceptionType>())
        QTest::newRow(c_str("transfer_Exception", name)) << ThrowLocation::Reader_transfer << value;
}

void TestIO::System_exportItemsWriterException_test()
{
    QFETCH(ThrowLocation, throwLocation);
    QFETCH(ExceptionType, exceptionType);

    IO::System ioSystem;
    IO::addPredefinedFormatProbes(&ioSystem);
    ioSystem.addFactoryWriter(
        std::make_unique<ThrowingFactoryWriter>(IO::Format::Format_OCCBREP, throwLocation, exceptionType)
    );

    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    MessageCollecter msgCollect;
    msgCollect.only(MessageType::Error);

    bool exportOk = false;
    try {
        exportOk = ioSystem.exportItems(
            IO::System::ArgsExport()
            .setTargetFile(FilePath{"test.brep"})
            .setTargetFormat(IO::Format::Format_OCCBREP)
            .setItem(ApplicationItem(doc))
            .setMessenger(&msgCollect)
        );
    }
    catch (...) {
        QFAIL("IO::System::exportItems() must not throw");
    }

    QVERIFY(!exportOk);
    QCOMPARE(msgCollect.messageCount(MessageType::Error), 1);
    if (exceptionType != ExceptionType::Other)
        QVERIFY(msgCollect.messages()[0].text.find(exceptionMsg(throwLocation)) != std::string::npos);
}

void TestIO::System_exportItemsWriterException_test_data()
{
    QTest::addColumn<ThrowLocation>("throwLocation");
    QTest::addColumn<ExceptionType>("exceptionType");

    auto c_str = [](std::string_view lhs, std::string_view rhs) {
        return staticStringStore(std::string{lhs}.append(rhs)).c_str();
    };

    for (const auto& [value, name] : MetaEnum::entries<ExceptionType>())
        QTest::newRow(c_str("writeFile_Exception", name)) << ThrowLocation::Writer_writeFile << value;

    for (const auto& [value, name] : MetaEnum::entries<ExceptionType>())
        QTest::newRow(c_str("transfer_Exception", name)) << ThrowLocation::Writer_transfer << value;
}

} // namespace Mayo
