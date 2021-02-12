/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_ptr.h"
#include "graphics_object_base_property_group.h"
#include "../base/enumeration.h"
#include "../base/property.h"
#include "../base/span.h"

#include <Standard_Transient.hxx>
#include <TDF_Label.hxx>
#include <memory>

namespace Mayo {

class GraphicsObjectDriver;
DEFINE_STANDARD_HANDLE(GraphicsObjectDriver, Standard_Transient)
using GraphicsObjectDriverPtr = opencascade::handle<GraphicsObjectDriver>;

class GraphicsObjectDriver : public Standard_Transient {
public:
    enum Support {
        None,
        Partial,
        Complete
    };

    virtual Support supportStatus(const TDF_Label& label) const = 0;

    virtual GraphicsObjectPtr createObject(const TDF_Label& label) const = 0;

    Enumeration::Value defaultDisplayMode() const { return m_defaultDisplayMode; }
    const Enumeration& displayModes() const { return m_enumDisplayModes; }
    virtual void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const = 0;
    virtual Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const = 0;

    virtual std::unique_ptr<GraphicsObjectBasePropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const = 0;

    static GraphicsObjectDriverPtr get(const GraphicsObjectPtr& object);
    static GraphicsObjectDriverPtr getCommon(Span<const GraphicsObjectPtr> spanObject);

    DEFINE_STANDARD_RTTI_INLINE(GraphicsObjectDriver, Standard_Transient)

protected:
    void setDisplayModes(Enumeration enumeration) { m_enumDisplayModes = std::move(enumeration); }
    void setDefaultDisplayMode(Enumeration::Value mode) { m_defaultDisplayMode = mode; }
    void throwIf_invalidDisplayMode(Enumeration::Value mode) const;
    void throwIf_differentDriver(const GraphicsObjectPtr& object) const;
    void throwIf_differentDriver(Span<const GraphicsObjectPtr> objects) const;

private:
    Enumeration m_enumDisplayModes;
    Enumeration::Value m_defaultDisplayMode = -1;
};

class GraphicsShapeObjectDriver : public GraphicsObjectDriver {
public:
    GraphicsShapeObjectDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsObjectPtr createObject(const TDF_Label& label) const override;
    void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const override;
    Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const override;
    std::unique_ptr<GraphicsObjectBasePropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const override;

    enum DisplayMode {
        DisplayMode_Wireframe,
        DisplayMode_HiddenLineRemoval,
        DisplayMode_Shaded,
        DisplayMode_ShadedWithFaceBoundary
    };
};

class GraphicsMeshObjectDriver : public GraphicsObjectDriver {
public:
    GraphicsMeshObjectDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsObjectPtr createObject(const TDF_Label& label) const override;
    void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const override;
    Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const override;
    std::unique_ptr<GraphicsObjectBasePropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const override;

    struct DefaultValues {
        bool showEdges = false;
        bool showNodes = false;
        Graphic3d_NameOfMaterial material = Graphic3d_NOM_PLASTIC;
        Quantity_Color color = Quantity_NOC_BISQUE;
        Quantity_Color edgeColor = Quantity_NOC_BLACK;
    };
    static const DefaultValues& defaultValues();
    static void setDefaultValues(const DefaultValues& values);

private:
    class ObjectProperties;
};

} // namespace Mayo
