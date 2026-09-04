/****************************************************************************
+** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
+** All rights reserved.
+** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
+****************************************************************************/

// --
// NOTE
// This file isolates inclusion of <Aspect_DisplayConnection.hxx> which is problematic on X11/Linux
// <X.h> #defines constants like "None" which causes name clash with GuiDocument::ViewTrihedronMode::None
// --

#include "../base/global.h"

#ifdef MAYO_OS_WINDOWS
#  include <windows.h>
#endif

#include "../base/occ_handle.h"

#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#if defined(MAYO_OS_WINDOWS)
#  include <WNT_WClass.hxx>
#  include <WNT_Window.hxx>
#elif defined(MAYO_OS_MAC) || defined(MAYO_OS_ANDROID)
#  include <Aspect_NeutralWindow.hxx>
#else
#  include <Xw_Window.hxx>
#endif

namespace Mayo {

OccHandle<Aspect_Window> graphicsCreateVirtualWindow(
        [[maybe_unused]]const OccHandle<Graphic3d_GraphicDriver>& gfxDriver, int wndWidth, int wndHeight
    )
{
#if defined(MAYO_OS_WINDOWS)
    // Create a "virtual" WNT window being a pure WNT window redefined to be never shown
    static auto wClass = makeOccHandle<WNT_WClass>(
        "GW3D_Class", nullptr, CS_VREDRAW | CS_HREDRAW, 0, 0, LoadCursor(nullptr, IDC_ARROW)
    );
    auto wnd = new WNT_Window("", wClass, WS_POPUP, 0, 0, wndWidth, wndHeight, Quantity_NOC_BLACK);
#elif defined(MAYO_OS_MAC) || defined(MAYO_OS_ANDROID)
    // Don't use Cocoa_Window on macOS: its constructor allocates a real on-screen NSWindow, which
    //   * throws Aspect_WindowDefinitionError when NSApp == nullptr (this may happen with mayo-conv
    //     being a QCoreApplication and never instantiates NSApplication)
    //   * must only ever be done on the main thread, while offscreen rendering typically happens
    //     in a worker thread
    // Use a neutral window for offscreen rendering : no native window is created
    // This is important on macOS, where Cocoa_Window woud create an NSWindow and require
    // NSApplication/main-thread usage
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
