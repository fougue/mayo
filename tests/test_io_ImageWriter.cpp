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
#include "../src/base/tkernel_utils.h"
#include "../src/base/thread_messenger_channel.h"
#include "../src/graphics/graphics_shape_object_driver.h"
#include "../src/gui/gui_application.h"
#include "../src/io_image/io_image.h"
#include "qttest_utils.h"

#include <QtTest/QtTest>

#include <Image_Diff.hxx>
#include <Image_PixMap.hxx>

#include <fmt/format.h>
#include <cmath>
#include <iostream>
#include <limits>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb/stb_image.h>

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

bool loadPixmap(const FilePath& filepath, Image_PixMap* pixmap)
{
    // Force 4 components : RGBA8
    int width, height, channels;
    unsigned char* src = stbi_load(filepath.u8string().c_str(), &width, &height, &channels, 4);
    if (!src)
        return false;

    if (!pixmap->InitTrash(Image_Format_RGBA, width, height))
        return false;

    const size_t size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    std::memcpy(pixmap->ChangeData(), src, size);
    stbi_image_free(src);
    return true;
}

#if 0
// "Root Mean Square" diff of two images
double imageRmsDiff(const FilePath& lhs, const FilePath& rhs)
{
    Image_PixMap pixmap1;
    Image_PixMap pixmap2;
    loadPixmap(lhs, &pixmap1);
    loadPixmap(rhs, &pixmap2);

    if (pixmap1.Width() != pixmap2.Width() || pixmap1.Height() != pixmap2.Height())
        return std::numeric_limits<double>::infinity();

    double sumSq = 0;
    const size_t count = pixmap1.Width() * pixmap1.Height() * 3;
    for (size_t i = 0; i < count; ++i) {
        const double diff = double(pixmap1.Data()[i]) - double(pixmap2.Data()[i]);
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / count);
}
#endif

bool imageCompare(const FilePath& lhs, const FilePath& rhs, bool hasGradientBackground)
{
    auto pixmap1 = makeOccHandle<Image_PixMap>();
    auto pixmap2 = makeOccHandle<Image_PixMap>();
    loadPixmap(lhs, pixmap1.get());
    loadPixmap(rhs, pixmap2.get());

    Image_Diff diff;
    if (!diff.Init(pixmap1, pixmap2))
        return false;

    if (hasGradientBackground) {
        // The border filter is designed for a uniform background and may incorrectly discard
        // differences against a non-uniform background
        // Allow minor GPU interpolation differences and a few pixels around the silhouette
        diff.SetColorTolerance(0.02);   // To calibrate ?
        diff.SetBorderFilterOn(false);
        return diff.Compare() <= 5;     // Allow up to 5 different pixels
    }
    else {
        // With a uniform background, the border filter effectively handles minor differences caused
        // by anti-aliasing and GPU-dependent rendering around the silhouette
        diff.SetColorTolerance(0.);
        diff.SetBorderFilterOn(true);
        return diff.Compare() == 0;
    }
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
    const auto gradientFill = static_cast<GradientFill>(gradientFillEnum);

    if (TKernelUtils::preferredRgbColorType() == Quantity_TOC_RGB)
        QSKIP("Requires OpenCascade >= 7.5.0 for stable/comparable gradient rendering (sRGB->linear RGB color space change)");

    if (gradientFill == GradientFill::Radial && !IO::ImageWriter::isRadialGradientFillSupported())
        QSKIP("Gradient fill not supported with this OpenCascade version");

    HelperTestImage helper;
    QVERIFY(m_ioSystem->importInDocument(helper.doc, "tests/inputs/cube.brep"));

    helper.params().backgroundGradientFill = gradientFill;
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
    QVERIFY(imageCompare(inputRefPath, outPath, gradientFill != GradientFill::None));
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

    if (TKernelUtils::preferredRgbColorType() == Quantity_TOC_RGB)
        QSKIP("Requires OpenCascade >= 7.5.0 for stable/comparable gradient rendering (sRGB->linear RGB color space change)");

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
    QVERIFY(imageCompare(inputRefPath, outPath, false/*blackBackground*/));
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
