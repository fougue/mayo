/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_scene.h"

#include <V3d_View.hxx>

namespace Mayo {

// Helper class providing a V3d view created associated with the owner GraphicsScene object
// The redraw() member function actually calls GraphicsScene::redraw() which under the hood sends
// a "redraw requested" signal
class GraphicsViewPtr {
public:
    GraphicsViewPtr(GraphicsScene* scene, const Handle_V3d_View& view)
        : m_scene(scene),
          m_view(view)
    {}

    const Handle_V3d_View& v3dView() const {
        return m_view;
    }

    GraphicsScene* scene() const {
        return m_scene;
    }

    void redraw() {
        m_scene->redraw(m_view);
    }

private:
    GraphicsScene* m_scene = nullptr;
    Handle_V3d_View m_view;
};

} // namespace Mayo
