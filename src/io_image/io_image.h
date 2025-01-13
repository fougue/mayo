/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include "../base/application_item.h"
#include "../base/caf_utils.h"
#include "../base/tkernel_utils.h"

#include <gp_Dir.hxx>
#include <Image_AlienPixMap.hxx>
#include <Quantity_Color.hxx>
#include <TDF_Label.hxx>
#include <V3d_View.hxx>

#include <vector>

// Pre-decls
namespace Mayo {
class GraphicsScene;
class GuiApplication;
class GuiDocument;
} // namespace Mayo

namespace Mayo {
namespace IO {

// Provides a writer for image creation
// Formats are those supported by OpenCascade with Image_AlienPixMap, see:
//     https://dev.opencascade.org/doc/refman/html/class_image___alien_pix_map.html#details
// The image format is specified with the extension for the target file path(eg .png, .jpeg, ...)
class ImageWriter : public Writer {
public:
    ImageWriter(GuiApplication* guiApp);

    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters
    enum class CameraProjection {
        Perspective, Orthographic
    };

    struct Parameters {
        int width = 128;
        int height = 128;
        Quantity_Color backgroundColor = Quantity_NOC_BLACK;
        gp_Vec cameraOrientation = gp_Vec(1, -1, 1); // X+ Y- Z+
        CameraProjection cameraProjection = CameraProjection::Orthographic;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    // Helper
    static OccHandle<Image_AlienPixMap> createImage(GuiDocument* guiDoc, const Parameters& params);
    static OccHandle<Image_AlienPixMap> createImage(OccHandle<V3d_View> view);
    static OccHandle<V3d_View> createV3dView(GraphicsScene* gfxScene, const Parameters& params);

private:
    class Properties;
    GuiApplication* m_guiApp = nullptr;
    Parameters m_params;
    std::vector<ApplicationItem> m_vecAppItem;
};

class ImageFactoryWriter : public FactoryWriter {
public:
    ImageFactoryWriter(GuiApplication* guiApp);
    Span<const Format> formats() const override;
    std::unique_ptr<Writer> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;

private:
    GuiApplication* m_guiApp = nullptr;
};

} // namespace IO
} // namespace Mayo
