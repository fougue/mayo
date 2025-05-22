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
#include "../graphics/graphics_object_driver.h"

#include <gp_Dir.hxx>
#include <Image_AlienPixMap.hxx>
#include <Quantity_Color.hxx>
#include <TDF_Label.hxx>
#include <V3d_View.hxx>

#include <map>
#include <optional>
#include <vector>

// Pre-decls
namespace Mayo {
class GraphicsScene;
class GuiApplication;
class GuiDocument;
} // namespace Mayo

namespace Mayo::IO {

// Provides a writer for image creation
// Formats are those supported by OpenCascade with Image_AlienPixMap, see:
//     https://dev.opencascade.org/doc/refman/html/class_image___alien_pix_map.html#details
// The image format is specified with the extension for the target file path(eg .png, .jpeg, ...)
class ImageWriter : public Writer {
public:
    ImageWriter(GuiApplication* guiApp);

    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(
        PropertyGroup* parentGroup, const GuiApplication* guiApp = nullptr
    );
    void applyProperties(const PropertyGroup* params) override;

    // Parameters
    enum class CameraProjection {
        Perspective, Orthographic
    };

    enum class GradientFill {
        // No gadient fill, single color background specified with Parameters::backgroundColorStart
        None,
        // Gradient directed from left(colorStart) to right(colorEnd)
        Horizontal,
        // Gradient directed from top(colorStart) to bottom(colorEnd)
        Vertical,
        // Gradient directed from top-left corner(colorStart) to bottom-right(colorEnd)
        DiagonalTopLeftBottomRight,
        // Gradient directed from top-right corner(colorStart) to bottom-left(colorEnd)
        DiagonalTopRightBottomLeft,
        // Gradient directed from center(colorStart) in all directions forming concentric circles
        // towards colorEnd
        Radial
    };

    struct Parameters {
        int width = 128;
        int height = 128;
        Quantity_Color backgroundColorStart = Quantity_NOC_BLACK;
        Quantity_Color backgroundColorEnd = Quantity_NOC_BLACK;
        GradientFill backgroundGradientFill = GradientFill::None;
        gp_Vec cameraOrientation = gp_Vec{1, -1, 1}; // X+ Y- Z+
        CameraProjection cameraProjection = CameraProjection::Orthographic;

        std::optional<Enumeration::Value> displayMode(const GraphicsObjectDriverPtr& driver) const;
        void setDisplayMode(const GraphicsObjectDriverPtr& driver, Enumeration::Value enumValue);

    private:
        std::map<GraphicsObjectDriverPtr, Enumeration::Value> m_driverDisplayModes;
        friend class ImageWriter;
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

} // namespace Mayo::IO
