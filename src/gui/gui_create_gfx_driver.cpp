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
#include <QtCore/QtGlobal>

namespace Mayo {
namespace Internal {

Handle_Graphic3d_GraphicDriver createGfxDriver()
{
    Handle_Aspect_DisplayConnection dispConnection;
#if (!defined(Q_OS_WIN32) && (!defined(Q_OS_MAC) || defined(MACOSX_USE_GLX)))
    dispConnection = new Aspect_DisplayConnection(std::getenv("DISPLAY"));
#endif
    Handle_Graphic3d_GraphicDriver gfxDriver = new OpenGl_GraphicDriver(dispConnection);
    return gfxDriver;
}

} // namespace Internal
} // namespace Mayo
