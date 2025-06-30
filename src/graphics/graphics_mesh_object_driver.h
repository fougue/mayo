/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_driver.h"

#include <MeshVS_DisplayModeFlags.hxx>

namespace Mayo {

class GraphicsMeshObjectDriver;
DEFINE_STANDARD_HANDLE(GraphicsMeshObjectDriver, GraphicsObjectDriver)
using GraphicsMeshObjectDriverPtr = OccHandle<GraphicsMeshObjectDriver>;

// Provides creation and configuration of graphics objects for meshes(triangulations)
class GraphicsMeshObjectDriver : public GraphicsObjectDriver {
public:
    GraphicsMeshObjectDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsObjectPtr createObject(const TDF_Label& label) const override;
    void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const override;
    Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const override;
    std::unique_ptr<PropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const override;

    static Support meshSupportStatus(const TDF_Label& label);

    struct DefaultValues {
        bool showEdges = false;
        bool showNodes = false;
        Graphic3d_NameOfMaterial material = Graphic3d_NOM_PLASTER;
        Quantity_Color color = Quantity_NOC_BISQUE;
        Quantity_Color edgeColor = Quantity_NOC_BLACK;
    };
    static const DefaultValues& defaultValues();
    static void setDefaultValues(const DefaultValues& values);

    enum DisplayMode {
        DisplayMode_Wireframe = MeshVS_DMF_WireFrame,
        DisplayMode_Shaded = MeshVS_DMF_Shading,
        DisplayMode_Shrink = MeshVS_DMF_Shrink
    };

    DEFINE_STANDARD_RTTI_INLINE(GraphicsMeshObjectDriver, GraphicsObjectDriver)

private:
    class ObjectProperties;
};

} // namespace Mayo
