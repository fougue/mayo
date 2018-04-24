#pragma once

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Aspect_Window.hxx>
#include <V3d_View.hxx>

namespace Mayo {

struct GpxUtils {
    static void V3dView_fitAll(const Handle_V3d_View& view);
    static bool V3dView_hasClipPlane(
            const Handle_V3d_View& view,
            const Handle_Graphic3d_ClipPlane& plane);

    static void AisContext_eraseObject(
            const Handle_AIS_InteractiveContext& context,
            const Handle_AIS_InteractiveObject& object);

    static int AspectWindow_width(const Handle_Aspect_Window& wnd);
    static int AspectWindow_height(const Handle_Aspect_Window& wnd);

    static void Gpx3dClipPlane_setCappingHatch(
            const Handle_Graphic3d_ClipPlane& plane, Aspect_HatchStyle hatch);
    static void Gpx3dClipPlane_setNormal(
            const Handle_Graphic3d_ClipPlane& plane, const gp_Dir& n);
    static void Gpx3dClipPlane_setPosition(
            const Handle_Graphic3d_ClipPlane& plane, double pos);
};

} // namespace Mayo
