/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_io.h"

#include "../src/base/application.h"
#include "../src/base/document.h"
#include "../src/base/filepath_conv.h"
#include "../src/base/io_system.h"
#include "../src/base/messenger.h"
#include "../src/base/meta_enum.h"
#include "../src/base/task_progress.h"
#include "../src/base/thread_messenger_channel.h"
#include "../src/graphics/graphics_shape_object_driver.h"
#include "../src/gui/gui_application.h"
#include "../src/io_image/io_image.h"

#include <QtTest/QtTest>

#include <Image_AlienPixMap.hxx>

#include <fmt/format.h>
#include <cmath>
#include <iostream>
#include <limits>

namespace Mayo {

namespace {

class StdOutMessenger : public Messenger {
public:
    void emitMessage(MessageType msgType, std::string_view text) override
    {
        if (msgType == MessageType::Warning)
            std::cerr << "[WARNING] " << text << std::endl;
        else if (msgType == MessageType::Error)
            std::cerr << "[ERROR] " << text << std::endl;
    }
};

// "Root Mean Square" diff of two images
double imageRmsDiff(const Image_PixMap& lhs, const Image_PixMap& rhs)
{
    if (lhs.Width() != rhs.Width() || lhs.Height() != rhs.Height())
        return std::numeric_limits<double>::infinity();

    double sumSq = 0;
    const size_t count = lhs.Width() * lhs.Height() * 3;
    for (size_t i = 0; i < count; ++i) {
        const double diff = double(lhs.Data()[i]) - double(rhs.Data()[i]);
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / count);
}

double imageRmsDiff(const std::filesystem::path& lhs, const std::filesystem::path& rhs)
{
    Image_AlienPixMap pixmap1;
    Image_AlienPixMap pixmap2;
    pixmap1.Load(filepathTo<TCollection_AsciiString>(lhs));
    pixmap2.Load(filepathTo<TCollection_AsciiString>(rhs));
    return imageRmsDiff(pixmap1, pixmap2);
}

struct HelperTestImage {
    ApplicationPtr app{makeOccHandle<Application>()};
    GuiApplication guiApp{app};
    DocumentPtr doc{app->newDocument()};
    IO::ImageWriter writer{&guiApp};
    StdOutMessenger messenger;

    HelperTestImage()
    {
        this->guiApp.addGraphicsObjectDriver(std::make_unique<GraphicsShapeObjectDriver>());
        this->writer.setMessenger(&this->messenger);
        this->writer.parameters().msaaSamples = IO::ImageWriter::MsaaSamples::Off;
    }

    IO::ImageWriter::Parameters& params()
    {
        return this->writer.parameters();
    }

    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress = nullptr)
    {
        [[maybe_unused]] ThreadMessengerChannel::Scope scopeMsg(&this->messenger);
        return this->writer.transfer(appItems, progress ? progress : &TaskProgress::null());
    }

    bool transferDocument(TaskProgress* progress = nullptr)
    {
        const ApplicationItem items[] = { ApplicationItem{this->doc} };
        return this->writer.transfer(items, progress ? progress : &TaskProgress::null());
    }

    bool writeFile(const std::filesystem::path& filepath, TaskProgress* progress = nullptr)
    {
        [[maybe_unused]] ThreadMessengerChannel::Scope scopeMsg(&this->messenger);
        return this->writer.writeFile(filepath, progress ? progress : &TaskProgress::null());
    }
};

} // namespace

void TestIO::ImageWriter_emptyTransfer_test()
{
    HelperTestImage helper;
    QVERIFY(helper.transfer({}));

    const std::filesystem::path outPath = "tests/outputs/ImageWriter_emptyTransfer_test.png";
    std::filesystem::remove(outPath);
    QVERIFY(helper.writeFile(outPath));
    QVERIFY(std::filesystem::exists(outPath));
}

void TestIO::ImageWriter_backgroundGradientFill_test()
{
    QFETCH(int, gradientFillEnum);

    using GradientFill = IO::ImageWriter::GradientFill;
    HelperTestImage helper;
    QVERIFY(m_ioSystem->importInDocument(helper.doc, "tests/inputs/cube.brep"));

    helper.params().backgroundGradientFill = static_cast<GradientFill>(gradientFillEnum);
    helper.params().backgroundColorStart = Quantity_NOC_RED1;
    helper.params().backgroundColorEnd = Quantity_NOC_BLUE1;
    QVERIFY(helper.transferDocument());

    const std::string strFileName = fmt::format(
        "ImageWriter_backgroundGradientFill_test_{}.png",
        MetaEnum::name<GradientFill>(helper.params().backgroundGradientFill)
    );
    const std::filesystem::path outPath = "tests/outputs/" + strFileName;
    std::filesystem::remove(outPath);
    QVERIFY(helper.writeFile(outPath));
    QVERIFY(std::filesystem::exists(outPath));

    const std::filesystem::path inputRefPath = "tests/inputs/refs/" + strFileName;
    QCOMPARE_LT(imageRmsDiff(outPath, inputRefPath), 0.5);
}

void TestIO::ImageWriter_backgroundGradientFill_test_data()
{
    using GradientFill = IO::ImageWriter::GradientFill;
    QTest::addColumn<int>("gradientFillEnum");
    QTest::newRow("None") << int(GradientFill::None);
    QTest::newRow("Horizontal") << int(GradientFill::Horizontal);
    QTest::newRow("Vertical") << int(GradientFill::Vertical);
    QTest::newRow("DiagonalTopLeftBottomRight") << int(GradientFill::DiagonalTopLeftBottomRight);
    QTest::newRow("DiagonalTopRightBottomLeft") << int(GradientFill::DiagonalTopRightBottomLeft);
    QTest::newRow("Radial") << int(GradientFill::Radial);
}

void TestIO::ImageWriter_writeValidPngFile_test()
{
    QFETCH(int, width);
    QFETCH(int, height);

    HelperTestImage helper;
    QVERIFY(m_ioSystem->importInDocument(helper.doc, "tests/inputs/cube.brep"));

    helper.params().width = width;
    helper.params().height = height;

    QVERIFY(helper.transferDocument());

    const std::string strFileName = fmt::format(
        "{}_output{}x{}.png", "ImageWriter_writeValidPngFile_test", width, height
    );
    const std::filesystem::path outPath = "tests/outputs/" + strFileName;
    std::filesystem::remove(outPath);
    QVERIFY(helper.writeFile(outPath));
    QVERIFY(std::filesystem::exists(outPath));
    QVERIFY(std::filesystem::file_size(outPath) > 0);

    const std::filesystem::path inputRefPath = "tests/inputs/refs/" + strFileName;
    QCOMPARE_LT(imageRmsDiff(outPath, inputRefPath), 0.5);
}

void TestIO::ImageWriter_writeValidPngFile_test_data()
{
    QTest::addColumn<int>("width");
    QTest::addColumn<int>("height");
    QTest::newRow("small") << 64 << 64;
    QTest::newRow("default") << 128 << 128;
    QTest::newRow("large") << 512 << 512;
}

} // namespace Mayo
