/****************************************************************************
+** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
+** All rights reserved.
+** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
+****************************************************************************/

// --
// NOTE
// This file isolates inclusion of <Aspect_DisplayConnection.hxx> which is problematic on X11/Linux
// <X.h> #defines constants like "None" which causes name clash with GuiDocument::ViewTrihedronMode::None
// --

#if defined(_WIN32)
#  include <windows.h>
#endif

#include "../base/global.h"

#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#if defined(_WIN32)
#  include <WNT_WClass.hxx>
#  include <WNT_Window.hxx>
#elif defined(__APPLE__)
#  include <Cocoa_Window.hxx>
#elif defined(__ANDROID__)
#  include <Aspect_NeutralWindow.hxx>
#else
#  include <Xw_Window.hxx>
#endif

namespace Mayo {

Handle_Aspect_Window graphicsCreateVirtualWindow(const Handle_Graphic3d_GraphicDriver& gfxDriver, int wndWidth, int wndHeight)
{
#if defined(_WIN32)
    MAYO_UNUSED(gfxDriver);
    // Create a "virtual" WNT window being a pure WNT window redefined to be never shown
    auto cursor = LoadCursor(NULL, IDC_ARROW);
    auto wClass = new WNT_WClass("GW3D_Class", (void*)DefWindowProcW, CS_VREDRAW | CS_HREDRAW, 0, 0, cursor);
    auto wnd = new WNT_Window("", wClass, WS_POPUP, 0, 0, wndWidth, wndHeight, Quantity_NOC_BLACK);
#elif defined(__APPLE__)
    MAYO_UNUSED(gfxDriver);
    auto wnd = new Cocoa_Window("", 0, 0, wndWidth, wndHeight);
#elif defined(__ANDROID__)
    MAYO_UNUSED(gfxDriver);
    auto wnd = new Aspect_NeutralWindow;
    wnd->SetSize(wndWidth, wndHeight);
#else
    auto displayConn = gfxDriver->GetDisplayConnection();
    auto wnd = new Xw_Window(displayConn, "", 0, 0, wndWidth, wndHeight);
#endif

    wnd->SetVirtual(true);
    return wnd;
}

} // namespace Mayo
