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

#include <gsl/util>
#include <limits>
#include <unordered_set>

namespace Mayo {

// Defined in graphics_create_virtual_window.cpp
OccHandle<Aspect_Window> graphicsCreateVirtualWindow(const OccHandle<Graphic3d_GraphicDriver>&, int , int);

namespace IO {

struct ImageWriterI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::ImageWriterI18N)
};

class ImageWriter::Properties : public PropertyGroup {
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->width.setDescription(ImageWriterI18N::textIdTr("Image width in pixels"));
        this->width.setConstraintsEnabled(true);
        this->width.setRange(0, std::numeric_limits<int>::max());

        this->height.setDescription(ImageWriterI18N::textIdTr("Image height in pixels"));
        this->height.setConstraintsEnabled(true);
        this->height.setRange(0, std::numeric_limits<int>::max());

        this->cameraOrientation.setDescription(
            ImageWriterI18N::textIdTr("Camera orientation expressed in Z-up convention as a unit vector")
        );
        this->cameraProjection.mutableEnumeration().changeTrContext(ImageWriterI18N::textIdContext());
    }

    void restoreDefaults() override
    {
        const Parameters defaults;
        this->width.setValue(defaults.width);
        this->height.setValue(defaults.height);
        this->backgroundColor.setValue(defaults.backgroundColor);
        this->cameraOrientation.setValue(defaults.cameraOrientation);
        this->cameraProjection.setValue(defaults.cameraProjection);
    }

    PropertyInt width{ this, ImageWriterI18N::textId("width") };
    PropertyInt height{ this, ImageWriterI18N::textId("height") };
    PropertyOccColor backgroundColor{ this, ImageWriterI18N::textId("backgroundColor") };
    PropertyOccVec cameraOrientation{ this, ImageWriterI18N::textId("cameraOrientation") };
    PropertyEnum<CameraProjection> cameraProjection{ this, ImageWriterI18N::textId("cameraProjection") };
};

namespace {

bool isVectorNull(const gp_Vec& vec)
{
    return vec.IsEqual({}, Precision::Confusion(), Precision::Angular());
}

} // namespace

ImageWriter::ImageWriter(GuiApplication* guiApp)
    : m_guiApp(guiApp)
{
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

    // Create 3D view
    GraphicsScene gfxScene;
    OccHandle<V3d_View> view = ImageWriter::createV3dView(&gfxScene, m_params);

    const int itemCount = CppUtils::safeStaticCast<int>(m_vecAppItem.size());
    // Render application items
    for (const ApplicationItem& appItem : m_vecAppItem) {
        if (appItem.isDocument()) {
            // Iterate other root entities
            const DocumentPtr doc = appItem.document();
            for (int i = 0; i < doc->entityCount(); ++i) {
                const TDF_Label labelEntity = doc->entityLabel(i);
                gfxScene.addObject(m_guiApp->createGraphicsObject(labelEntity));
            }
        }
        else if (appItem.isDocumentTreeNode()) {
            const TDF_Label labelNode = appItem.documentTreeNode().label();
            gfxScene.addObject(m_guiApp->createGraphicsObject(labelNode));
        }

        const auto itemProgress = &appItem - &m_vecAppItem.front();
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

std::unique_ptr<PropertyGroup> ImageWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void ImageWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.width = ptr->width;
        m_params.height = ptr->height;
        m_params.backgroundColor = ptr->backgroundColor;
        m_params.cameraOrientation = ptr->cameraOrientation;
        m_params.cameraProjection = ptr->cameraProjection;
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
    view->SetBackgroundColor(params.backgroundColor);
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
        return ImageWriter::createProperties(parentGroup);

    return {};
}

} // namespace IO
} // namespace Mayo
