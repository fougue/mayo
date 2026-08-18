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

#ifdef MAYO_OS_MAC
#  include <OpenGl_Context.hxx>
#  include <OpenGl_GraphicDriver.hxx>
#endif

namespace Mayo {

OccHandle<Aspect_Window> graphicsCreateVirtualWindow(
        [[maybe_unused]]const OccHandle<Graphic3d_GraphicDriver>& gfxDriver, int wndWidth, int wndHeight
    )
{
#if defined(MAYO_OS_WINDOWS)
    // Create a "virtual" WNT window being a pure WNT window redefined to be never shown
    static OccHandle<WNT_WClass> wClass;
    if (wClass.IsNull()) {
        auto cursor = LoadCursor(NULL, IDC_ARROW);
        wClass = new WNT_WClass("GW3D_Class", nullptr, CS_VREDRAW | CS_HREDRAW, 0, 0, cursor);
    }

    auto wnd = new WNT_Window("", wClass, WS_POPUP, 0, 0, wndWidth, wndHeight, Quantity_NOC_BLACK);
#elif defined(MAYO_OS_MAC) || defined(MAYO_OS_ANDROID)
    // Don't use Cocoa_Window on macOS: its constructor allocates a real on-screen NSWindow, which
    //     * throws Aspect_WindowDefinitionError when NSApp == nullptr(eg. in mayo-conv, which is a
    //       QCoreApplication and so never instantiates NSApplication)
    //     * must only ever be done on the main thread, while offscreen rendering typically happens
    //       in a worker thread
    // Aspect_NeutralWindow is just a size/position holder without any native window attached, which
    // is all that is needed: as the window is flagged "virtual", OpenGl_Window doesn't try to bind
    // the GL context to a native drawable and V3d_View::ToPixMap() renders into an FBO
    auto wnd = new Aspect_NeutralWindow;
    wnd->SetSize(wndWidth, wndHeight);
#else
    auto displayConn = gfxDriver->GetDisplayConnection();
    auto wnd = new Xw_Window(displayConn, "", 0, 0, wndWidth, wndHeight);
#endif

    wnd->SetVirtual(true);
    return wnd;
}

void graphicsPrepareVirtualWindowRendering(
        [[maybe_unused]]const OccHandle<Graphic3d_GraphicDriver>& gfxDriver
    )
{
#ifdef MAYO_OS_MAC
    // On macOS the GL context of a virtual window has no drawable attached to it, so its default
    // framebuffer is incomplete. Any GL call targeting it(eg. glDrawBuffer(GL_BACK), issued by
    // OpenGl_Window::init()) raises GL_INVALID_FRAMEBUFFER_OPERATION. OpenCascade doesn't reset that
    // pending error, and later on OpenGl_Texture::Init() reads it back while creating the FBO used
    // for the offscreen dump: it wrongly concludes that texture creation failed, making
    // V3d_View::ToPixMap() bail out. Flushing the GL error queue beforehand avoids this false
    // negative(rendering into an FBO from a drawable-less context works fine otherwise)
    auto occGfxDriver = OccHandle<OpenGl_GraphicDriver>::DownCast(gfxDriver);
    if (occGfxDriver.IsNull())
        return;

    const OccHandle<OpenGl_Context>& gfxContext = occGfxDriver->GetSharedContext();
    if (gfxContext.IsNull())
        return;

    // Prefer the context being already current: in the desktop application the graphics driver is
    // shared with the on-screen view, so avoid stealing "current" from it needlessly
    if (gfxContext->IsCurrent() || gfxContext->MakeCurrent())
        gfxContext->ResetErrors(false/*!ToPrintErrors*/);
#endif
}

} // namespace Mayo
