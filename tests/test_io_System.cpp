/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_io.h"

#include "../src/base/application.h"
#include "../src/base/message_collecter.h"
#include "../src/base/io_system.h"
#include "signal_emit_spy.h"

#include <QtTest/QtTest>

#include <string>

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(Mayo::IO::Format)
Q_DECLARE_METATYPE(std::string)

namespace Mayo {

namespace {

class ThrowingReader : public IO::Reader {
public:
    enum class ThrowLocation { ReadFile, Transfer };

    explicit ThrowingReader(ThrowLocation throwLocation)
        : m_throwLocation(throwLocation)
    { }

    bool readFile(const FilePath&, TaskProgress*) override
    {
        if (m_throwLocation == ThrowLocation::ReadFile)
            throw std::runtime_error("Exception in Reader::readFile()");
        return true;
    }

    NCollection_Sequence<TDF_Label> transfer(DocumentPtr, TaskProgress*) override
    {
        if (m_throwLocation == ThrowLocation::Transfer)
            throw std::runtime_error("Exception in Reader::transfer()");
        return {};
    }

    void applyProperties(const PropertyGroup*) override
    { }

private:
    ThrowLocation m_throwLocation;
};

class ThrowingWriter : public IO::Writer {
public:
    enum class ThrowLocation { WriteFile, Transfer };

    explicit ThrowingWriter(ThrowLocation throwLocation)
        : m_throwLocation(throwLocation)
    { }

    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override
    {
        if (m_throwLocation == ThrowLocation::Transfer)
            throw std::runtime_error("Exception in Writer::transfer()");
        return {};
    }

    bool writeFile(const FilePath&, TaskProgress*) override
    {
        if (m_throwLocation == ThrowLocation::WriteFile)
            throw std::runtime_error("Exception in Writer::writeFile()");
        return true;
    }

    void applyProperties(const PropertyGroup*) override
    { }

private:
    ThrowLocation m_throwLocation;
};

class ThrowingFactoryReader : public IO::FactoryReader {
public:
    ThrowingFactoryReader(IO::Format format, ThrowingReader::ThrowLocation throwLocation)
        : m_format(format),
          m_throwLocation(throwLocation)
    { }

    gsl::span<const IO::Format> formats() const override
    {
        return gsl::span<const IO::Format>(&m_format, 1);
    }

    std::unique_ptr<IO::Reader> create(IO::Format format) const override
    {
        if (format != m_format)
            return {};
        return std::make_unique<ThrowingReader>(m_throwLocation);
    }

    std::unique_ptr<PropertyGroup> createProperties(IO::Format, PropertyGroup*) const override
    {
        return {};
    }

private:
    IO::Format m_format;
    ThrowingReader::ThrowLocation m_throwLocation;
};

class ThrowingFactoryWriter : public IO::FactoryWriter{
public:
    ThrowingFactoryWriter(IO::Format format, ThrowingWriter::ThrowLocation throwLocation)
        : m_format(format),
          m_throwLocation(throwLocation)
    { }

    gsl::span<const IO::Format> formats() const override
    {
        return gsl::span<const IO::Format>(&m_format, 1);
    }

    std::unique_ptr<IO::Writer> create(IO::Format format) const override
    {
        if (format != m_format)
            return {};
        return std::make_unique<ThrowingWriter>(m_throwLocation);
    }

    std::unique_ptr<PropertyGroup> createProperties(IO::Format, PropertyGroup*) const override
    {
        return {};
    }

private:
    IO::Format m_format;
    ThrowingWriter::ThrowLocation m_throwLocation;
};

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
    const bool okImport = m_ioSystem->importInDocument(
        IO::System::ArgsImport()
            .setTargetDocument(doc)
            .setFilepath("tests/inputs/#268_ambient-intensity-outofrange.wrl")
            .setMessenger(&msgCollect)
        );
    QVERIFY(!okImport);
    const size_t errCount = std::count_if(
        msgCollect.messages().begin(),
        msgCollect.messages().end(),
        [](const Messenger::Message& msg) { return msg.type == MessageType::Error; }
    );
    QCOMPARE(errCount, 1u);
    // Should be message "Error in VrmlAPI_CafReader: IrrelevantNumberoccurred at line (...)"
    QVERIFY(msgCollect.messages()[0].text.find("VrmlAPI_CafReader") != std::string::npos);
#endif
}

void TestIO::System_importInDocumentReaderException_test()
{
    using ThrowLocation = ThrowingReader::ThrowLocation;
    for (const auto throwLocation : { ThrowLocation::ReadFile, ThrowLocation::Transfer }) {
        IO::System ioSystem;
        IO::addPredefinedFormatProbes(&ioSystem);
        ioSystem.addFactoryReader(
            std::make_unique<ThrowingFactoryReader>(IO::Format::Format_OCCBREP, throwLocation)
        );

        auto app = makeOccHandle<Application>();
        DocumentPtr doc = app->newDocument();

        bool importOk = false;
        try {
            importOk = ioSystem.importInDocument(doc, FilePath{"test.brep"});
        }
        catch (...) {
            QFAIL("IO::System::importInDocument() must not throw");
        }

        QVERIFY(!importOk);
    }
}

void TestIO::System_exportItemsWriterException_test()
{
    using ThrowLocation = ThrowingWriter::ThrowLocation;
    for (const auto throwLocation : { ThrowLocation::WriteFile, ThrowLocation::Transfer }) {
        IO::System ioSystem;
        IO::addPredefinedFormatProbes(&ioSystem);
        ioSystem.addFactoryWriter(
            std::make_unique<ThrowingFactoryWriter>(IO::Format::Format_OCCBREP, throwLocation)
        );

        auto app = makeOccHandle<Application>();
        DocumentPtr doc = app->newDocument();

        bool exportOk = false;
        try {
            exportOk = ioSystem.exportItems(
                IO::System::ArgsExport()
                .setTargetFile(FilePath{"test.brep"})
                .setTargetFormat(IO::Format::Format_OCCBREP)
                .setItem(ApplicationItem(doc))
            );
        }
        catch (...) {
            QFAIL("IO::System::exportItems() must not throw");
        }

        QVERIFY(!exportOk);
    }
}

} // namespace Mayo
