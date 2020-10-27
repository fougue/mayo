/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <AIS_InteractiveObject.hxx>
#include <TDF_Label.hxx>

namespace Mayo {

class GraphicsEntityDriver;
class GraphicsScene;

class GraphicsEntity {
public:
    GraphicsEntity() = default;

    const TDF_Label& label() const { return m_label; }

    bool aisObjectNotNull() const { return !m_aisObject.IsNull(); }
    bool sceneNotNull() const;

    const Handle_AIS_InteractiveObject& aisObject() const { return m_aisObject; }
    AIS_InteractiveObject* aisObjectPtr() const { return m_aisObject.get(); }

    GraphicsScene* graphicsScene() const { return m_gfxScene; }
    void setScene(GraphicsScene* scene) { m_gfxScene = scene; }

    const GraphicsEntityDriver* driverPtr() const { return m_driverPtr; }

    bool isVisible() const;
    void setVisible(bool on);

    int displayMode() const;
    void setDisplayMode(int mode);

private:
    friend class GraphicsEntityDriver;
    Handle_AIS_InteractiveObject m_aisObject;
    TDF_Label m_label;
    GraphicsScene* m_gfxScene = nullptr;
    const GraphicsEntityDriver* m_driverPtr = nullptr;
};

} // namespace Mayo
