/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_entity.h"
#include "../base/property_enumeration.h"
#include <QtCore/QCoreApplication>

namespace Mayo {

class GraphicsEntityDriver {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GraphicsEntityDriver)
public:
    enum Support {
        None,
        Partial,
        Complete
    };

    virtual Support supportStatus(const TDF_Label& label) const = 0;

    virtual GraphicsEntity createEntity(const TDF_Label& label) const = 0;

    const Enumeration& displayModes() const { return m_enumDisplayModes; }
    virtual void applyDisplayMode(const GraphicsEntity& entity, Enumeration::Value mode) const = 0;

protected:
    void setDisplayModes(const Enumeration& enumeration) { m_enumDisplayModes = enumeration; }
    void throwIf_invalidDisplayMode(Enumeration::Value mode) const;

    void initEntity(GraphicsEntity* ptrEntity, const TDF_Label& label) const;
    static void setEntityAisObject(GraphicsEntity* ptrEntity, const Handle_AIS_InteractiveObject& obj);

private:
    Enumeration m_enumDisplayModes;
};

class GraphicsShapeEntityDriver : public GraphicsEntityDriver {

public:
    GraphicsShapeEntityDriver();

    Support supportStatus(const TDF_Label& label) const override;
    GraphicsEntity createEntity(const TDF_Label& label) const override;
    void applyDisplayMode(const GraphicsEntity& entity, Enumeration::Value mode) const override;

protected:
    enum DisplayMode {
        DisplayMode_Wireframe,
        DisplayMode_Shaded,
        DisplayMode_ShadedWithFaceBoundary
    };
};

class GraphicsMeshEntityDriver : public GraphicsEntityDriver {
public:
    GraphicsMeshEntityDriver();
    Support supportStatus(const TDF_Label& label) const override;
    GraphicsEntity createEntity(const TDF_Label& label) const override;
    void applyDisplayMode(const GraphicsEntity& entity, Enumeration::Value mode) const override;
};

} // namespace Mayo
