/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include "../base/application_item.h"
#include "../base/caf_utils.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
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
    explicit ImageWriter(GuiApplication* guiApp);

    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

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

    struct Parameters : public PropertyGroup {
        PropertyInt width{ this, textId("width") };
        PropertyInt height{ this, textId("height") };
        PropertyOccColor backgroundColorStart{ this, textId("backgroundColorStart") };
        PropertyOccColor backgroundColorEnd{ this, textId("backgroundColorEnd") };
        PropertyEnum<GradientFill> backgroundGradientFill{ this, textId("backgroundGradientFill") };
        PropertyOccVec cameraOrientation{ this, textId("cameraOrientation") };
        PropertyEnum<CameraProjection> cameraProjection{ this, textId("cameraProjection") };

        std::optional<Enumeration::Value> displayMode(const GraphicsObjectDriverPtr& driver) const;
        void setDisplayMode(const GraphicsObjectDriverPtr& driver, Enumeration::Value enumValue);

        Parameters(const GuiApplication* guiApp);
        void restoreDefaults() override;

    private:
        std::map<GraphicsObjectDriverPtr, std::unique_ptr<PropertyEnumeration>> m_mapDriverDisplayMode;
        friend class ImageWriter;
    };

    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

    // Helper
    static OccHandle<Image_AlienPixMap> createImage(GuiDocument* guiDoc, const Parameters& params);
    static OccHandle<Image_AlienPixMap> createImage(OccHandle<V3d_View> view);
    static OccHandle<V3d_View> createV3dView(GraphicsScene* gfxScene, const Parameters& params);

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::ImageWriter)

    GuiApplication* m_guiApp = nullptr;
    Parameters m_params;
    std::vector<ApplicationItem> m_vecAppItem;
};

class ImageFactoryWriter : public FactoryWriter {
public:
    explicit ImageFactoryWriter(GuiApplication* guiApp);
    gsl::span<const Format> formats() const override;
    std::unique_ptr<Writer> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createParameters(Format format) const override;

private:
    GuiApplication* m_guiApp = nullptr;
};

} // namespace Mayo::IO
