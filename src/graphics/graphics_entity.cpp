/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_entity.h"

#include "../gpx/gpx_utils.h"

#include <gsl/gsl_assert>
#include <Standard_Version.hxx>

namespace Mayo {

bool GraphicsEntity::aisContextNotNull() const
{
    return this->aisObjectNotNull() && m_aisObject->HasInteractiveContext();
}

Handle_AIS_InteractiveContext GraphicsEntity::aisContext() const
{
    Expects(this->aisObjectNotNull());
    return m_aisObject->GetContext();
}

AIS_InteractiveContext* GraphicsEntity::aisContextPtr() const
{
    Expects(this->aisObjectNotNull());
#if OCC_VERSION_HEX >= 0x070400
    return m_aisObject->InteractiveContext();
#else
    return m_aisObject->GetContext().get();
#endif
}

void GraphicsEntity::setAisContext(const Handle_AIS_InteractiveContext& context)
{
    Expects(this->aisObjectNotNull());
    Expects(!m_aisObject->HasInteractiveContext());

    m_aisObject->SetContext(context);

    Ensures(this->aisContextNotNull());
}

bool GraphicsEntity::isVisible() const
{
    if (this->aisContextNotNull())
        return this->aisContextPtr()->IsDisplayed(this->aisObject());
    else
        return false;
}

void GraphicsEntity::setVisible(bool on)
{
    Expects(this->aisContextNotNull());
    GpxUtils::AisContext_setObjectVisible(this->aisContext(), this->aisObject(), on);
}

} // namespace Mayo
