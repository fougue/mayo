/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gpx_utils.h"
#include "../base/math_utils.h"

#include <algorithm>
#include <ElSLib.hxx>
#include <ProjLib.hxx>
#include <SelectMgr_SelectionManager.hxx>

namespace Mayo {

void GpxUtils::V3dView_fitAll(const Handle_V3d_View& view)
{
    view->ZFitAll();
    view->FitAll(0.01, false);
}

bool GpxUtils::V3dView_hasClipPlane(
        const Handle_V3d_View &view, const Handle_Graphic3d_ClipPlane& plane)
{
    const Handle_Graphic3d_SequenceOfHClipPlane& seqClipPlane = view->ClipPlanes();
    if (seqClipPlane.IsNull() || seqClipPlane->Size() == 0)
        return false;
    for (const Handle_Graphic3d_ClipPlane& candidate : *seqClipPlane) {
        if (candidate.operator->() == plane.operator->())
            return true;
    }
    return false;
}

gp_Pnt GpxUtils::V3dView_to3dPosition(
        const Handle_V3d_View& view, double x, double y)
{
    double xEye, yEye, zEye, xAt, yAt, zAt;
    view->Eye(xEye, yEye, zEye);
    view->At(xAt, yAt, zAt);
    const gp_Pnt pntEye(xEye, yEye, zEye);
    const gp_Pnt pntAt(xAt, yAt, zAt);

    const gp_Vec vecEye(pntEye, pntAt);
    const gp_Dir dirEye(vecEye);

    const gp_Pln planeView(pntAt, dirEye);
    double px, py, pz;
    const int ix = static_cast<int>(std::round(x));
    const int iy = static_cast<int>(std::round(y));
    view->Convert(ix, iy, px, py, pz);
    const gp_Pnt pntConverted(px, py, pz);
    const gp_Pnt2d pntConvertedOnPlane = ProjLib::Project(planeView, pntConverted);
    const gp_Pnt pntResult =
            ElSLib::Value(
                pntConvertedOnPlane.X(),
                pntConvertedOnPlane.Y(),
                planeView);
    return pntResult;
}

void GpxUtils::AisContext_eraseObject(
        const Handle_AIS_InteractiveContext& context,
        const Handle_AIS_InteractiveObject& object)
{
    if (!object.IsNull() && !context.IsNull()) {
        context->Erase(object, false);
        context->Remove(object, false);
        context->ClearPrs(object, 0, false);
        context->SelectionManager()->Remove(object);

        Handle_AIS_InteractiveObject objectHCopy = object;
        while (!objectHCopy.IsNull())
            objectHCopy.Nullify();
    }
}

void GpxUtils::AisContext_setObjectVisible(
        const Handle_AIS_InteractiveContext& context,
        const Handle_AIS_InteractiveObject& object,
        bool on)
{
    if (!context.IsNull() && !object.IsNull()) {
        if (on)
            context->Display(object, false);
        else
            context->Erase(object, false);
    }
}

int GpxUtils::AspectWindow_width(const Handle_Aspect_Window& wnd)
{
    if (wnd.IsNull())
        return 0;
    int w, h;
    wnd->Size(w, h);
    return w;
}

int GpxUtils::AspectWindow_height(const Handle_Aspect_Window& wnd)
{
    if (wnd.IsNull())
        return 0;
    int w, h;
    wnd->Size(w, h);
    return h;
}

void GpxUtils::Gpx3dClipPlane_setCappingHatch(
        const Handle_Graphic3d_ClipPlane& plane, Aspect_HatchStyle hatch)
{
    if (hatch == Aspect_HS_SOLID)
        plane->SetCappingHatchOff();
    else
        plane->SetCappingHatchOn();
    plane->SetCappingHatch(hatch);
}

void GpxUtils::Gpx3dClipPlane_setNormal(
        const Handle_Graphic3d_ClipPlane& plane, const gp_Dir &n)
{
    const double planePos = MathUtils::planePosition(plane->ToPlane());
    const gp_Vec placement(planePos * gp_Vec(n));
    plane->SetEquation(gp_Pln(placement.XYZ(), n));
}

void GpxUtils::Gpx3dClipPlane_setPosition(
        const Handle_Graphic3d_ClipPlane& plane, double pos)
{
    const gp_Dir& n = plane->ToPlane().Axis().Direction();
    if (MathUtils::isReversedStandardDir(n))
        pos = -pos;
    const gp_Vec placement(pos * gp_Vec(n));
    plane->SetEquation(gp_Pln(placement.XYZ(), n));
}

} // namespace Mayo
