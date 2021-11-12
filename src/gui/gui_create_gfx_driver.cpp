/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

// --
// NOTE
// This file isolates inclusion of <Aspect_DisplayConnection.hxx> which is problematic on X11/Linux
// <X.h> #defines constants like "None" which causes name clash with GuiDocument::ViewTrihedronMode::None
// --

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_Version.hxx>
#include <QtCore/QtGlobal>

namespace Mayo {
namespace Internal {

Handle_Graphic3d_GraphicDriver createGfxDriver()
{
    Handle_Aspect_DisplayConnection dispConnection;
#if (!defined(Q_OS_WIN) && (!defined(Q_OS_MAC) || defined(MACOSX_USE_GLX)))
    dispConnection = new Aspect_DisplayConnection(std::getenv("DISPLAY"));
#else
    dispConnection = new Aspect_DisplayConnection;
#endif

#if OCC_VERSION_HEX >= 0x070600
    auto gfxDriver = new OpenGl_GraphicDriver(dispConnection, false/*dontInit*/);
    // Let QOpenGLWidget to manage buffer swap
    gfxDriver->ChangeOptions().buffersNoSwap = true;
    // Don't write into alpha channel
    gfxDriver->ChangeOptions().buffersOpaqueAlpha = true;
    // Offscreen FBOs should be always used
    gfxDriver->ChangeOptions().useSystemBuffer = false;
#  if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    Message::SendWarning("Warning! Qt 5.10+ is required for sRGB setup.\n"
                         "Colors in 3D Viewer might look incorrect (Qt " QT_VERSION_STR " is used).\n");
    gfxDriver->ChangeOptions().sRGBDisable = true;
#  endif
#else
    auto gfxDriver = new OpenGl_GraphicDriver(dispConnection);
#endif

    return gfxDriver;
}

} // namespace Internal
} // namespace Mayo
