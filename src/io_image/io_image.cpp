/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_image.h"

#include "../base/application_item.h"
#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
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
Handle_Aspect_Window graphicsCreateVirtualWindow(const Handle_Graphic3d_GraphicDriver&, int , int);

namespace IO {

class ImageWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::ImageWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->width.setDescription(textIdTr("Image width in pixels"));
        this->width.setConstraintsEnabled(true);
        this->width.setRange(0, std::numeric_limits<int>::max());

        this->height.setDescription(textIdTr("Image height in pixels"));
        this->height.setConstraintsEnabled(true);
        this->height.setRange(0, std::numeric_limits<int>::max());

        this->cameraOrientation.setDescription(
                    textIdTr("Camera orientation expressed in Z-up convention as a unit vector"));
    }

    void restoreDefaults() override {
        const Parameters defaults;
        this->width.setValue(defaults.width);
        this->height.setValue(defaults.height);
        this->backgroundColor.setValue(defaults.backgroundColor);
        this->cameraOrientation.setValue(defaults.cameraOrientation);
        this->cameraProjection.setValue(defaults.cameraProjection);
    }

    PropertyInt width{ this, textId("width") };
    PropertyInt height{ this, textId("height") };
    PropertyOccColor backgroundColor{ this, textId("backgroundColor") };
    PropertyOccVec cameraOrientation{ this, textId("cameraOrientation") };
    PropertyEnum<CameraProjection> cameraProjection{ this, textId("cameraProjection") };
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
    m_setDoc.clear();
    m_setNode.clear();

    for (const ApplicationItem& item : appItems) {
        if (item.isDocument())
            m_setDoc.insert(item.document());
    }

    for (const ApplicationItem& item : appItems) {
        if (item.isDocumentTreeNode()) {
            auto itDoc = m_setDoc.find(item.document());
            if (itDoc == m_setDoc.cend())
                m_setNode.insert(item.documentTreeNode().label());
        }
    }

    return true;
}

bool ImageWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (isVectorNull(m_params.cameraOrientation))
        this->messenger()->emitError(ImageWriter::Properties::textIdTr("Camera orientation vector must not be null"));

    // Create 3D view
    GraphicsScene gfxScene;
    Handle_V3d_View view = ImageWriter::createV3dView(&gfxScene, m_params);

    int itemProgress = 0;
    const int itemCount = m_setDoc.size() + m_setNode.size();
    // Render documents(iterate other root entities)
    for (const DocumentPtr& doc : m_setDoc) {
        for (int i = 0; i < doc->entityCount(); ++i) {
            const TDF_Label labelEntity = doc->entityLabel(i);
            GraphicsObjectPtr gfxObject = m_guiApp->graphicsObjectDriverTable()->createObject(labelEntity);
            gfxScene.addObject(gfxObject);
        }

        progress->setValue(MathUtils::mappedValue(++itemProgress, 0, itemCount, 0, 100));
    }

    // Render document tree nodes
    for (const TDF_Label& labelNode : m_setNode) {
        GraphicsObjectPtr gfxObject = m_guiApp->graphicsObjectDriverTable()->createObject(labelNode);
        gfxScene.addObject(gfxObject);
        progress->setValue(MathUtils::mappedValue(++itemProgress, 0, itemCount, 0, 100));
    }

    gfxScene.redraw();
    GraphicsUtils::V3dView_fitAll(view);
    Handle_Image_AlienPixMap pixmap = ImageWriter::createImage(view);
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

Handle_Image_AlienPixMap ImageWriter::createImage(GuiDocument* guiDoc, const Parameters& params)
{
    if (!guiDoc)
        return {};

    const GuiDocument::ViewTrihedronMode onEntryTrihedronMode = guiDoc->viewTrihedronMode();
    const bool onEntryOriginTrihedronVisible = guiDoc->isOriginTrihedronVisible();
    Handle_V3d_View view = ImageWriter::createV3dView(guiDoc->graphicsScene(), params);

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

Handle_Image_AlienPixMap ImageWriter::createImage(Handle_V3d_View view)
{
    Handle_Image_AlienPixMap pixmap = new Image_AlienPixMap;
    V3d_ImageDumpOptions dumpOptions;
    dumpOptions.BufferType = Graphic3d_BT_RGB;
    view->Window()->Size(dumpOptions.Width, dumpOptions.Height);
    const bool okPixmap = view->ToPixMap(*pixmap.get(), dumpOptions);
    if (!okPixmap)
        return {};

    pixmap->SetFormat(Image_Format_RGB);
    return pixmap;
}

Handle_V3d_View ImageWriter::createV3dView(GraphicsScene* gfxScene, const Parameters& params)
{
    auto fnToGfxCamProjection = [](ImageWriter::CameraProjection proj) {
        switch (proj) {
        case CameraProjection::Orthographic: return Graphic3d_Camera::Projection_Orthographic;
        case CameraProjection::Perspective:  return Graphic3d_Camera::Projection_Perspective;
        default: return Graphic3d_Camera::Projection_Orthographic;
        }
    };

    // Create 3D view
    Handle_V3d_View view = gfxScene->createV3dView();
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
