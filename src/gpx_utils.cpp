#include "gpx_utils.h"
#include "math_utils.h"

#include <algorithm>
#include <SelectMgr_SelectionManager.hxx>

namespace Mayo {

void GpxUtils::V3dView_fitAll(const Handle_V3d_View& view)
{
    view->ZFitAll();
    view->FitAll();
}

bool GpxUtils::V3dView_hasClipPlane(
        const Handle_V3d_View &view, const Handle_Graphic3d_ClipPlane &plane)
{
    const Handle_Graphic3d_SequenceOfHClipPlane& seqClipPlane = view->ClipPlanes();
    if (seqClipPlane.IsNull() || seqClipPlane->Size() == 0)
        return false;
    auto itFound = std::find_if(
                seqClipPlane->cbegin(),
                seqClipPlane->cend(),
                [=](const Handle_Graphic3d_ClipPlane& candidate) {
       return candidate.operator->() == plane.operator->();
    });
    return itFound != seqClipPlane->cend();
}

void GpxUtils::AisContext_eraseObject(
        const Handle_AIS_InteractiveContext& context,
        const Handle_AIS_InteractiveObject& object)
{
    if (!object.IsNull()) {
        context->Erase(object, false);
        context->Remove(object, false);
        context->ClearPrs(object, 0, false);
        context->SelectionManager()->Remove(object);

        Handle_AIS_InteractiveObject objectHCopy = object;
        while (!objectHCopy.IsNull())
            objectHCopy.Nullify();
    }
}

int GpxUtils::AspectWindow_width(const Handle_Aspect_Window &wnd)
{
    if (wnd.IsNull())
        return 0;
    int w, h;
    wnd->Size(w, h);
    return w;
}

int GpxUtils::AspectWindow_height(const Handle_Aspect_Window &wnd)
{
    if (wnd.IsNull())
        return 0;
    int w, h;
    wnd->Size(w, h);
    return h;
}

void GpxUtils::Gpx3dClipPlane_setCappingHatch(
        const Handle_Graphic3d_ClipPlane &plane, Aspect_HatchStyle hatch)
{
    if (hatch == Aspect_HS_SOLID)
        plane->SetCappingHatchOff();
    else
        plane->SetCappingHatchOn();
    plane->SetCappingHatch(hatch);
}

void GpxUtils::Gpx3dClipPlane_setNormal(
        const Handle_Graphic3d_ClipPlane &plane, const gp_Dir &n)
{
    const double planePos = MathUtils::planePosition(plane->ToPlane());
    const gp_Vec placement(planePos * gp_Vec(n));
    plane->SetEquation(gp_Pln(placement.XYZ(), n));
}

void GpxUtils::Gpx3dClipPlane_setPosition(
        const Handle_Graphic3d_ClipPlane &plane, double pos)
{
    const gp_Dir& n = plane->ToPlane().Axis().Direction();
    if (MathUtils::isReversedStandardDir(n))
        pos = -pos;
    const gp_Vec placement(pos * gp_Vec(n));
    plane->SetEquation(gp_Pln(placement.XYZ(), n));
}

} // namespace Mayo
