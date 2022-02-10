/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_utils.h"
#include "../base/bnd_utils.h"
#include "../base/math_utils.h"
#include "../base/tkernel_utils.h"

#include <Bnd_Box.hxx>
#include <ElSLib.hxx>
#include <Image_PixMap.hxx>
#include <ProjLib.hxx>
#include <SelectMgr_SelectionManager.hxx>
#include <Standard_Version.hxx>

#include <algorithm>
#include <cstring>

namespace Mayo {

namespace Internal {

static void AisContext_setObjectVisible(
        AIS_InteractiveContext* ptrContext, const Handle_AIS_InteractiveObject& object, bool on)
{
    if (ptrContext && object) {
        if (on)
            ptrContext->Display(object, false);
        else
            ptrContext->Erase(object, false);
    }
}

} // namespace Internal

void GraphicsUtils::V3dView_fitAll(const Handle_V3d_View& view)
{
    view->ZFitAll();
    view->FitAll(0.01, false);
}

bool GraphicsUtils::V3dView_hasClipPlane(
        const Handle_V3d_View& view, const Handle_Graphic3d_ClipPlane& plane)
{
    const Handle_Graphic3d_SequenceOfHClipPlane& seqClipPlane = view->ClipPlanes();
    if (seqClipPlane.IsNull() || seqClipPlane->Size() == 0)
        return false;

    for (Graphic3d_SequenceOfHClipPlane::Iterator it(*seqClipPlane); it.More(); it.Next()) {
        const Handle_Graphic3d_ClipPlane& candidate = it.Value();
        if (candidate.get() == plane.get())
            return true;
    }

    return false;
}

gp_Pnt GraphicsUtils::V3dView_to3dPosition(const Handle_V3d_View& view, double x, double y)
{
    double xEye, yEye, zEye, xAt, yAt, zAt;
    view->Eye(xEye, yEye, zEye);
    view->At(xAt, yAt, zAt);
    const gp_Pnt pntEye(xEye, yEye, zEye);
    const gp_Pnt pntAt(xAt, yAt, zAt);

    const gp_Vec vecEye(pntEye, pntAt);
    const bool vecEyeNotNull = vecEye.SquareMagnitude() > gp::Resolution();
    const gp_Dir dirEye(vecEyeNotNull ? vecEye : gp_Vec{0, 0, 1});

    const gp_Pln planeView(pntAt, dirEye);
    double px, py, pz;
    const int ix = static_cast<int>(std::round(x));
    const int iy = static_cast<int>(std::round(y));
    view->Convert(ix, iy, px, py, pz);
    const gp_Pnt pntConverted(px, py, pz);
    const gp_Pnt2d pntConvertedOnPlane = ProjLib::Project(planeView, pntConverted);
    return ElSLib::Value(pntConvertedOnPlane.X(), pntConvertedOnPlane.Y(), planeView);
}

void GraphicsUtils::AisContext_eraseObject(
        const Handle_AIS_InteractiveContext& context,
        const Handle_AIS_InteractiveObject& object)
{
    if (!object.IsNull() && !context.IsNull()) {
        context->Erase(object, false);
        context->Remove(object, false);
        context->ClearPrs(object, 0, false);
        context->SelectionManager()->Remove(object);
    }
}

void GraphicsUtils::AisContext_setObjectVisible(
        const Handle_AIS_InteractiveContext& context,
        const Handle_AIS_InteractiveObject& object,
        bool on)
{
    Internal::AisContext_setObjectVisible(context.get(), object, on);
}

AIS_InteractiveContext* GraphicsUtils::AisObject_contextPtr(const GraphicsObjectPtr& object)
{
    if (!object)
        return nullptr;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    return object->InteractiveContext();
#else
    return object->GetContext().get();
#endif
}

bool GraphicsUtils::AisObject_isVisible(const GraphicsObjectPtr& object)
{
    const AIS_InteractiveContext* ptrContext = AisObject_contextPtr(object);
    return ptrContext ? ptrContext->IsDisplayed(object) : false;
}

void GraphicsUtils::AisObject_setVisible(const GraphicsObjectPtr& object, bool on)
{
    Internal::AisContext_setObjectVisible(AisObject_contextPtr(object), object, on);
}

Bnd_Box GraphicsUtils::AisObject_boundingBox(const GraphicsObjectPtr& object)
{
    Bnd_Box box;
    if (object.IsNull())
        return box;

    // Ensure bounding box is calculated
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    for (Handle_PrsMgr_Presentation prs : object->Presentations()) {
        if (prs->Mode() == object->DisplayMode() && !prs->CStructure()->BoundingBox().IsValid())
            prs->CalculateBoundBox();
    }
#else
    for (PrsMgr_ModedPresentation& pres : object->Presentations()) {
        if (pres.Mode() == object->DisplayMode()) {
            const Handle_Prs3d_Presentation& pres3d = pres.Presentation()->Presentation();
            if (!pres3d->CStructure()->BoundingBox().IsValid())
                pres3d->CalculateBoundBox();
        }
    }
#endif

    object->BoundingBox(box);
    return box;
}

int GraphicsUtils::AspectWindow_width(const Handle_Aspect_Window& wnd)
{
    if (wnd.IsNull())
        return 0;

    int w, h;
    wnd->Size(w, h);
    return w;
}

int GraphicsUtils::AspectWindow_height(const Handle_Aspect_Window& wnd)
{
    if (wnd.IsNull())
        return 0;

    int w, h;
    wnd->Size(w, h);
    return h;
}

void GraphicsUtils::Gpx3dClipPlane_setCappingHatch(
        const Handle_Graphic3d_ClipPlane& plane, Aspect_HatchStyle hatch)
{
    if (hatch == Aspect_HS_SOLID)
        plane->SetCappingHatchOff();
    else
        plane->SetCappingHatchOn();

    plane->SetCappingHatch(hatch);
}

void GraphicsUtils::Gpx3dClipPlane_setNormal(const Handle_Graphic3d_ClipPlane& plane, const gp_Dir& n)
{
    const double planePos = MathUtils::planePosition(plane->ToPlane());
    const gp_Vec placement(planePos * gp_Vec(n));
    plane->SetEquation(gp_Pln(placement.XYZ(), n));
}

void GraphicsUtils::Gpx3dClipPlane_setPosition(const Handle_Graphic3d_ClipPlane& plane, double pos)
{
    const gp_Dir& n = plane->ToPlane().Axis().Direction();
    if (MathUtils::isReversedStandardDir(n))
        pos = -pos;

    const gp_Vec placement(pos * gp_Vec(n));
    plane->SetEquation(gp_Pln(placement.XYZ(), n));
}

bool GraphicsUtils::Image_flipY(Image_PixMap& pixmap)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    return Image_PixMap::FlipY(pixmap);
#else
    // Excerpt from OpenCascade7.6/src/Image/Image_Pixmap.cpp

    if (pixmap.IsEmpty() || pixmap.SizeX() == 0 || pixmap.SizeY() == 0)
        return false;

    NCollection_Buffer tmp(NCollection_BaseAllocator::CommonBaseAllocator());
    const size_t rowSize = pixmap.SizeRowBytes();
    if (!tmp.Allocate(rowSize))
        return false;

    // For odd height middle row should be left as is
    const Standard_Size nbRowsHalf = pixmap.SizeY() / 2;
    for (Standard_Size rowT = 0, rowB = pixmap.SizeY() - 1; rowT < nbRowsHalf; ++rowT, --rowB) {
        Standard_Byte* top = pixmap.ChangeRow(rowT);
        Standard_Byte* bottom = pixmap.ChangeRow(rowB);
        std::memcpy(tmp.ChangeData(), top, rowSize);
        std::memcpy(top, bottom, rowSize);
        std::memcpy(bottom, tmp.Data(), rowSize);
    }

    return true;
#endif
}

} // namespace Mayo
