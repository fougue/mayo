/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_object_ptr.h"
#include "../base/enumeration.h"
#include "../base/label_data.h"
#include "../base/property.h"
#include "../base/span.h"
#include "../base/text_id.h"

#include <Standard_Transient.hxx>
#include <TDF_Label.hxx>
#include <memory>

namespace Mayo {

class GraphicsObjectDriver;
DEFINE_STANDARD_HANDLE(GraphicsObjectDriver, Standard_Transient)
using GraphicsObjectDriverPtr = OccHandle<GraphicsObjectDriver>;

// Provides creation and configuration of graphics objects of a specific type
// Each graphics object "knows" the driver which created it: use function GraphicsObjectDriver::get()
class GraphicsObjectDriver : public Standard_Transient {
public:
    enum class Support { None, Partial, Complete };

    virtual Support supportStatus(const TDF_Label& label) const = 0;
    virtual GraphicsObjectPtr createObject(const TDF_Label& label) const = 0;

    Enumeration::Value defaultDisplayMode() const { return m_defaultDisplayMode; }
    const Enumeration& displayModes() const { return m_enumDisplayModes; }
    virtual void applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const = 0;
    virtual Enumeration::Value currentDisplayMode(const GraphicsObjectPtr& object) const = 0;

    virtual std::unique_ptr<PropertyGroup> properties(Span<const GraphicsObjectPtr> spanObject) const = 0;

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

struct GraphicsObjectDriverI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsObjectDriverI18N) };

} // namespace Mayo
