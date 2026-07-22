/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"

#include <QtGui/QSurfaceFormat>

#include <Aspect_Drawable.hxx>
#include <Graphic3d_Vec2.hxx>
#include <OpenGl_Context.hxx>
#include <V3d_View.hxx>
class OpenGl_Caps;

namespace Mayo::QtOpenGlUtils {

// Perform global Qt platform setup - to be called before QCoreApplication creation
// Defines platform plugin to load(eg xcb on Linux) and graphic driver(eg desktop OpenGL with
// desired profile/surface).
void platformSetup(int argc, char* argv[]);

// Define default Qt surface format for GL context
QSurfaceFormat surfaceFormat(QSurfaceFormat::OpenGLContextProfile profile, bool debug = false);

// OpenCascade GL caps from Qt surface format
void setCapsFromSurfaceFormat(OpenGl_Caps& caps, const QSurfaceFormat& format);


#if OCC_VERSION_HEX >= 0x070600

// Return active native window bound to OpenGL context
Aspect_Drawable glNativeWindow(Aspect_Drawable nativeWin);

OccHandle<OpenGl_Context> glContext(const OccHandle<V3d_View>& view);

// Cleanup up global GL state after Qt before redrawing OpenCascade view
void resetGlStateBeforeOcct(const OccHandle<V3d_View>& view);

// Cleanup up global GL state after OCCT before redrawing Qt
void resetGlStateAfterOcct(const OccHandle<V3d_View>& view);

// Initialize native window for OpenCascade view
bool initializeGlWindow(
    const OccHandle<V3d_View>& view, Aspect_Drawable nativeWin, const NCollection_Vec2<int>& size, double pixelRatio
);

// Wrap FBO created by QOpenGLFramebufferObject to OpenCascade viewe target
bool initializeGlFramebufferObject(const OccHandle<V3d_View>& view);

#endif // OCC_VERSION_HEX >= 0x070600

} // namespace Mayo::QtOpenGlUtils
