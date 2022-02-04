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
#include <Quantity_Color.hxx>
#include <TDF_Label.hxx>
#include <unordered_set>

// Pre-decls
namespace Mayo { class GuiApplication; }

namespace Mayo {
namespace IO {

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

private:
    class Properties;
    GuiApplication* m_guiApp = nullptr;
    Parameters m_params;
    std::unordered_set<DocumentPtr> m_setDoc;
    std::unordered_set<TDF_Label> m_setNode;
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
