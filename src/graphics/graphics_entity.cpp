/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_entity.h"

#include "../graphics/graphics_scene.h"

#include <gsl/gsl_assert>

namespace Mayo {

bool GraphicsEntity::sceneNotNull() const
{
    return m_gfxScene != nullptr;
}

bool GraphicsEntity::isVisible() const
{
    if (this->sceneNotNull())
        return m_gfxScene->isObjectVisible(m_aisObject);
    else
        return false;
}

void GraphicsEntity::setVisible(bool on)
{
    Expects(this->sceneNotNull());
    m_gfxScene->setObjectVisible(this->aisObject(), on);
}

int GraphicsEntity::displayMode() const
{
    Expects(this->aisObjectNotNull());
    return m_aisObject->DisplayMode();
}

void GraphicsEntity::setDisplayMode(int mode)
{
    Expects(this->sceneNotNull());
    m_gfxScene->setObjectDisplayMode(m_aisObject, mode);
}

} // namespace Mayo
