/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070600

#include "../base/occ_handle.h"

#include <Aspect_Drawable.hxx>
#include <NCollection_Vec2.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <V3d_View.hxx>

#include <functional>

namespace Mayo::OpenGlUtils {

// Return active native window bound to OpenGL context
Aspect_Drawable glNativeWindow(Aspect_Drawable nativeWin);

// Returns the OpenGL context associated with the given OpenCascade view
OccHandle<OpenGl_Context> glContext(const OccHandle<V3d_View>& view);

// Initialize native window for OpenCascade view
bool initializeGlWindow(
    const OccHandle<V3d_View>& view, Aspect_Drawable nativeWin, const NCollection_Vec2<int>& size, double pixelRatio
);

// Wrap already created FBO to OpenCascade viewer target
bool initializeGlFramebufferObject(
    const OccHandle<V3d_View>& view,
    const std::function<OccHandle<OpenGl_FrameBuffer>()>& createFbo
);

} // namespace Mayo::QtOpenGlUtils

#endif // OCC_VERSION_HEX >= 0x070600
