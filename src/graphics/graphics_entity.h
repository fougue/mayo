/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <TDF_Label.hxx>

namespace Mayo {

class GraphicsEntityDriver;

class GraphicsEntity {
public:
    // using AisObjectHandleType = const Handle_AIS_InteractiveObject&;
    // using AisObjectHandleType = Handle_AIS_InteractiveObject;

    GraphicsEntity() = default;

    const TDF_Label& label() const { return m_label; }

    bool aisObjectNotNull() const { return !m_aisObject.IsNull(); }
    bool aisContextNotNull() const;

    const Handle_AIS_InteractiveObject& aisObject() const { return m_aisObject; }
    AIS_InteractiveObject* aisObjectPtr() const { return m_aisObject.get(); }

    Handle_AIS_InteractiveContext aisContext() const;
    AIS_InteractiveContext* aisContextPtr() const;
    void setAisContext(const Handle_AIS_InteractiveContext& context);

    const GraphicsEntityDriver* driverPtr() const { return m_driverPtr; }

    bool isVisible() const;
    void setVisible(bool on);

private:
    friend class GraphicsEntityDriver;
    Handle_AIS_InteractiveObject m_aisObject;
    TDF_Label m_label;
    const GraphicsEntityDriver* m_driverPtr = nullptr;
};

} // namespace Mayo
