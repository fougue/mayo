/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "graphics_scene.h"

#include <V3d_View.hxx>

namespace Mayo {

// Helper class providing a V3d view associated with the owner GraphicsScene object
// The redraw() member function actually calls GraphicsScene::redraw() which under the hood sends
// a "redraw requested" signal
class GraphicsViewPtr {
public:
    GraphicsViewPtr(GraphicsScene* scene, const OccHandle<V3d_View>& view)
        : m_scene(scene),
          m_view(view)
    {}

    const OccHandle<V3d_View>& v3dView() const {
        return m_view;
    }

    GraphicsScene* scene() const {
        return m_scene;
    }

    void redraw() {
        m_scene->redraw(m_view);
    }

    const OccHandle<V3d_View>& operator->() const { return m_view; }

private:
    GraphicsScene* m_scene = nullptr;
    OccHandle<V3d_View> m_view;
};

} // namespace Mayo
