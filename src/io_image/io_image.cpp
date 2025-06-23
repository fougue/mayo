/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_image.h"

#include "../base/application_item.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/io_system.h"
#include "../base/math_utils.h"
#include "../base/messenger.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"
#include "../graphics/graphics_scene.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_application.h"

#include <Aspect_Window.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <Image_AlienPixMap.hxx>
#include <V3d_View.hxx>

#include <fmt/format.h>
#include <gsl/util>
#include <limits>

namespace Mayo {

// Defined in graphics_create_virtual_window.cpp
OccHandle<Aspect_Window> graphicsCreateVirtualWindow(const OccHandle<Graphic3d_GraphicDriver>&, int , int);

namespace IO {

struct ImageWriterI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::ImageWriterI18N)
};

class ImageWriter::Properties : public PropertyGroup {
public:
    Properties(PropertyGroup* parentGroup, const GuiApplication* guiApp)
        : PropertyGroup(parentGroup)
    {
        this->width.setDescription(ImageWriterI18N::textIdTr("Image width in pixels"));
        this->width.setConstraintsEnabled(true);
        this->width.setRange(0, std::numeric_limits<int>::max());

        this->height.setDescription(ImageWriterI18N::textIdTr("Image height in pixels"));
        this->height.setConstraintsEnabled(true);
        this->height.setRange(0, std::numeric_limits<int>::max());

        this->backgroundColorStart.setDescription(ImageWriterI18N::textIdTr(
            "Start color of the image background gradient"
        ));
        this->backgroundColorEnd.setDescription(ImageWriterI18N::textIdTr(
            "End color of the image background gradient"
        ));
        this->backgroundGradientFill.setDescription(ImageWriterI18N::textIdTr(
            "Type of gradient fill for the image background"
        ));
        this->backgroundGradientFill.setDescriptions({
            { GradientFill::None, ImageWriterI18N::textIdTr("No gadient fill, single color background") },
            { GradientFill::Horizontal, ImageWriterI18N::textIdTr("Gradient directed from left to right") },
            { GradientFill::Vertical, ImageWriterI18N::textIdTr("Gradient directed from top to bottom") },
            { GradientFill::DiagonalTopLeftBottomRight, ImageWriterI18N::textIdTr("Gradient directed from top left corner to bottom right") },
            { GradientFill::DiagonalTopRightBottomLeft, ImageWriterI18N::textIdTr("Gradient directed from top right corner to bottom left") },
            { GradientFill::Radial, ImageWriterI18N::textIdTr("Gradient directed from center in all directions forming concentric circles") },
        });
        this->backgroundGradientFill.mutableEnumeration().changeTrContext(ImageWriterI18N::textIdContext());

        this->cameraOrientation.setDescription(ImageWriterI18N::textIdTr(
            "Camera orientation expressed in Z-up convention as a unit vector"
        ));
        this->cameraProjection.setDescription(ImageWriterI18N::textIdTr(
            "Camera projection type, specifies how the 3D scene is projected onto a 2D image for display"
        ));
        this->cameraProjection.mutableEnumeration().changeTrContext(ImageWriterI18N::textIdContext());

        if (guiApp) {
            // Create a PropertyEnumeration object for each graphics driver registered in the given
            // GuiApplication object
            // This PropertyEnumeration is mapped to the display modes specific to the driver
            m_textIdStringStorage.reserve(guiApp->graphicsObjectDrivers().size());
            for (const GraphicsObjectDriverPtr& driver : guiApp->graphicsObjectDrivers()) {
                if (driver->displayModes().empty())
                    continue; // Skip

                const std::string driverTypeName = driver->DynamicType()->Name();
                const std::string trDriverTypeName{GraphicsObjectDriverI18N::textIdTr(driverTypeName)};
                m_textIdStringStorage.push_back(fmt::format("{}_displayMode", driver->DynamicType()->Name()));
                auto propDisplayMode = std::make_unique<PropertyEnumeration>(
                    this, ImageWriterI18N::textId(m_textIdStringStorage.back()), &driver->displayModes()
                );
                auto msgDescription = ImageWriterI18N::textIdTr("Graphics display mode for the objects of type `{}`");
                propDisplayMode->setDescription(fmt::format(msgDescription, trDriverTypeName));
                this->mapDriverDisplayMode.insert({ driver, std::move(propDisplayMode) });
            }
        }
    }

    ~Properties()
    {
    }

    void restoreDefaults() override
    {
        const Parameters defaults;
        this->width.setValue(defaults.width);
        this->height.setValue(defaults.height);
        this->backgroundColorStart.setValue(defaults.backgroundColorStart);
        this->backgroundColorEnd.setValue(defaults.backgroundColorEnd);
        this->backgroundGradientFill.setValue(defaults.backgroundGradientFill);
        this->cameraOrientation.setValue(defaults.cameraOrientation);
        this->cameraProjection.setValue(defaults.cameraProjection);
        for (const auto& [driver, propDisplayMode] : this->mapDriverDisplayMode) {
            if (!driver->displayModes().empty())
                propDisplayMode->setValue(driver->defaultDisplayMode());
        }
    }

    PropertyInt width{ this, ImageWriterI18N::textId("width") };
    PropertyInt height{ this, ImageWriterI18N::textId("height") };
    PropertyOccColor backgroundColorStart{ this, ImageWriterI18N::textId("backgroundColorStart") };
    PropertyOccColor backgroundColorEnd{ this, ImageWriterI18N::textId("backgroundColorEnd") };
    PropertyEnum<GradientFill> backgroundGradientFill{ this, ImageWriterI18N::textId("backgroundGradientFill") };
    PropertyOccVec cameraOrientation{ this, ImageWriterI18N::textId("cameraOrientation") };
    PropertyEnum<CameraProjection> cameraProjection{ this, ImageWriterI18N::textId("cameraProjection") };
    std::map<GraphicsObjectDriverPtr, std::unique_ptr<PropertyEnumeration>> mapDriverDisplayMode;

private:
    std::vector<std::string> m_textIdStringStorage;
};

namespace {

bool isVectorNull(const gp_Vec& vec)
{
    return vec.IsEqual({}, Precision::Confusion(), Precision::Angular());
}

Aspect_GradientFillMethod toOccGradientFill(ImageWriter::GradientFill fill)
{
    switch (fill) {
    case ImageWriter::GradientFill::None:
        return Aspect_GFM_NONE;
    case ImageWriter::GradientFill::Horizontal:
        return Aspect_GFM_HOR;
    case ImageWriter::GradientFill::Vertical:
        return Aspect_GFM_VER;
    case ImageWriter::GradientFill::DiagonalTopLeftBottomRight:
        return Aspect_GFM_DIAG1;
    case ImageWriter::GradientFill::DiagonalTopRightBottomLeft:
        return Aspect_GFM_DIAG2;
    case ImageWriter::GradientFill::Radial:
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        return Aspect_GradientFillMethod_Elliptical;
#else
        return Aspect_GFM_NONE;
#endif
    } // endswitch()

    return Aspect_GFM_NONE;
}

} // namespace

ImageWriter::ImageWriter(GuiApplication* guiApp)
    : m_guiApp(guiApp)
{
    for (const GraphicsObjectDriverPtr& driver : guiApp->graphicsObjectDrivers()) {
        if (!driver->displayModes().empty())
            m_params.m_driverDisplayModes.insert({ driver, driver->defaultDisplayMode() });
    }
}

bool ImageWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
    m_vecAppItem.clear();
    System::visitUniqueItems(appItems, [&](const ApplicationItem& item) { m_vecAppItem.push_back(item); });
    if (m_vecAppItem.empty())
        this->messenger()->emitWarning(ImageWriterI18N::textIdTr("No transferred application items"));

    return true;
}

bool ImageWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (isVectorNull(m_params.cameraOrientation))
        this->messenger()->emitError(ImageWriterI18N::textIdTr("Camera orientation vector must not be null"));

#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 6, 0)
    if (m_params.backgroundGradientFill == GradientFill::Radial) {
        this->messenger()->emitWarning(ImageWriterI18N::textIdTr(
            "Background radial gradient fill is available since OpenCascade 7.6.\n"
            "Default to background single color"
        ));
    }
#endif

    // Create 3D view
    GraphicsScene gfxScene;
    OccHandle<V3d_View> view = ImageWriter::createV3dView(&gfxScene, m_params);

    auto fnMapGraphicsObject = [&](const TDF_Label& labelEntity) {
        auto driver = m_guiApp->findCompatibleGraphicsObjectDriver(labelEntity);
        if (driver) {
            auto gfxObject = driver->createObject(labelEntity);
            gfxScene.addObject(gfxObject);
            auto itDisplayMode = m_params.m_driverDisplayModes.find(driver);
            if (itDisplayMode != m_params.m_driverDisplayModes.cend())
                driver->applyDisplayMode(gfxObject, itDisplayMode->second);
        }
    };

    const int itemCount = CppUtils::safeStaticCast<int>(m_vecAppItem.size());
    // Render application items
    for (const ApplicationItem& appItem : m_vecAppItem) {
        if (appItem.isDocument()) {
            // Iterate other root entities
            const DocumentPtr doc = appItem.document();
            for (int i = 0; i < doc->entityCount(); ++i) {
                const TDF_Label labelEntity = doc->entityLabel(i);
                fnMapGraphicsObject(labelEntity);
            }
        }
        else if (appItem.isDocumentTreeNode()) {
            const TDF_Label labelNode = appItem.documentTreeNode().label();
            fnMapGraphicsObject(labelNode);
        }

        const auto itemProgress = Span_itemIndex(m_vecAppItem, appItem);
        progress->setValue(MathUtils::toPercent(itemProgress, 0, itemCount));
    }

    view->Redraw();
    GraphicsUtils::V3dView_fitAll(view);
    OccHandle<Image_AlienPixMap> pixmap = ImageWriter::createImage(view);
    if (!pixmap)
        return false;

    const bool okSave = pixmap->Save(filepathTo<TCollection_AsciiString>(filepath));
    return okSave;
}

std::unique_ptr<PropertyGroup> ImageWriter::createProperties(
        PropertyGroup* parentGroup, const GuiApplication* guiApp
    )
{
    return std::make_unique<Properties>(parentGroup, guiApp);
}

void ImageWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.width = ptr->width;
        m_params.height = ptr->height;
        m_params.backgroundColorStart = ptr->backgroundColorStart;
        m_params.backgroundColorEnd = ptr->backgroundColorEnd;
        m_params.backgroundGradientFill = ptr->backgroundGradientFill;
        m_params.cameraOrientation = ptr->cameraOrientation;
        m_params.cameraProjection = ptr->cameraProjection;

        m_params.m_driverDisplayModes.clear();
        for (const auto& [driver, propDisplayMode] : ptr->mapDriverDisplayMode)
            m_params.m_driverDisplayModes.insert({ driver, propDisplayMode->value()});
    }
}

OccHandle<Image_AlienPixMap> ImageWriter::createImage(GuiDocument* guiDoc, const Parameters& params)
{
    if (!guiDoc)
        return {};

    const GuiDocument::ViewTrihedronMode onEntryTrihedronMode = guiDoc->viewTrihedronMode();
    const bool onEntryOriginTrihedronVisible = guiDoc->isOriginTrihedronVisible();
    OccHandle<V3d_View> view = ImageWriter::createV3dView(guiDoc->graphicsScene(), params);

    auto _ = gsl::finally([=]{
        guiDoc->graphicsScene()->v3dViewer()->SetViewOff(view);
        guiDoc->setViewTrihedronMode(onEntryTrihedronMode);
        if (guiDoc->isOriginTrihedronVisible() != onEntryOriginTrihedronVisible)
            guiDoc->toggleOriginTrihedronVisibility();
    });

    guiDoc->graphicsScene()->clearSelection();
    guiDoc->setViewTrihedronMode(GuiDocument::ViewTrihedronMode::None);
    if (guiDoc->isOriginTrihedronVisible())
        guiDoc->toggleOriginTrihedronVisibility();

    GraphicsUtils::V3dView_fitAll(view);
    return ImageWriter::createImage(view);
}

OccHandle<Image_AlienPixMap> ImageWriter::createImage(OccHandle<V3d_View> view)
{
    auto pixmap = makeOccHandle<Image_AlienPixMap>();
    V3d_ImageDumpOptions dumpOptions;
    dumpOptions.BufferType = Graphic3d_BT_RGB;
    view->Window()->Size(dumpOptions.Width, dumpOptions.Height);
    const bool okPixmap = view->ToPixMap(*pixmap.get(), dumpOptions);
    if (!okPixmap)
        return {};

    pixmap->SetFormat(Image_Format_RGB);
    return pixmap;
}

OccHandle<V3d_View> ImageWriter::createV3dView(GraphicsScene* gfxScene, const Parameters& params)
{
    auto fnToGfxCamProjection = [](ImageWriter::CameraProjection proj) {
        switch (proj) {
        case CameraProjection::Orthographic: return Graphic3d_Camera::Projection_Orthographic;
        case CameraProjection::Perspective:  return Graphic3d_Camera::Projection_Perspective;
        default: return Graphic3d_Camera::Projection_Orthographic;
        }
    };

    // Create 3D view
    OccHandle<V3d_View> view = gfxScene->createV3dView();
    view->ChangeRenderingParams().IsAntialiasingEnabled = true;
    view->ChangeRenderingParams().NbMsaaSamples = 4;
    if (params.backgroundGradientFill == GradientFill::None) {
        view->SetBackgroundColor(params.backgroundColorStart);
    }
    else {
        view->SetBgGradientColors(
            params.backgroundColorStart,
            params.backgroundColorEnd,
            toOccGradientFill(params.backgroundGradientFill),
            false/*!ToUpdate*/
        );
    }

    view->Camera()->SetProjectionType(fnToGfxCamProjection(params.cameraProjection));
    if (!isVectorNull(params.cameraOrientation))
        view->SetProj(params.cameraOrientation.X(), params.cameraOrientation.Y(), params.cameraOrientation.Z());
    else
        view->SetProj(1, -1, 1);

    // Create virtual window
    auto wnd = graphicsCreateVirtualWindow(view->Viewer()->Driver(), params.width, params.height);
    view->SetWindow(wnd);

    return view;
}

std::optional<Enumeration::Value>
ImageWriter::Parameters::displayMode(const GraphicsObjectDriverPtr& driver) const
{
    auto it = m_driverDisplayModes.find(driver);
    if (it != m_driverDisplayModes.cend())
        return it->second;

    return std::nullopt;
}

void ImageWriter::Parameters::setDisplayMode(const GraphicsObjectDriverPtr& driver, Enumeration::Value enumValue)
{
    auto it = m_driverDisplayModes.find(driver);
    if (it != m_driverDisplayModes.end())
        it->second = enumValue;
}


ImageFactoryWriter::ImageFactoryWriter(GuiApplication* guiApp)
    : m_guiApp(guiApp)
{
}

Span<const Format> ImageFactoryWriter::formats() const
{
    static const Format arrayFormat[] = { Format_Image };
    return arrayFormat;
}

std::unique_ptr<Writer> ImageFactoryWriter::create(Format format) const
{
    if (format == Format_Image)
        return std::make_unique<ImageWriter>(m_guiApp);

    return {};
}

std::unique_ptr<PropertyGroup> ImageFactoryWriter::createProperties(Format format, PropertyGroup* parentGroup) const
{
    if (format == Format_Image)
        return ImageWriter::createProperties(parentGroup, m_guiApp);

    return {};
}

} // namespace IO
} // namespace Mayo
