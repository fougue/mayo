/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"

#include <Aspect_HatchStyle.hxx>
#include <Bnd_Box.hxx>
#include <Quantity_Color.hxx>

// Note: can't include Aspect_DisplayConnection.hxx as this is causing name conflicts 
// with XLib in other files(with GraphicsObjectDriver::Support::None)
// This seems to happen only for OpenCascade <= v7.5
class Aspect_DisplayConnection;
class Aspect_Grid;
class Aspect_Window;
class AIS_InteractiveContext;
class AIS_InteractiveObject;
class Graphic3d_ClipPlane;
class Image_PixMap;
class V3d_View;
class V3d_Viewer;
class gp_Dir;

namespace Mayo {

struct GraphicsUtils {

    struct AspectGridColors {
        Quantity_Color base;
        Quantity_Color tenth;
    };

    static void V3dView_fitAll(const OccHandle<V3d_View>& view);
    static void V3dView_fitAll(const OccHandle<V3d_View>& view, const Bnd_Box& bndBox);
    static bool V3dView_hasClipPlane(const OccHandle<V3d_View>& view, const OccHandle<Graphic3d_ClipPlane>& plane);
    static gp_Pnt V3dView_to3dPosition(const OccHandle<V3d_View>& view, double x, double y);

    static bool V3dViewer_isGridActive(const OccHandle<V3d_Viewer>& viewer);
    static OccHandle<Aspect_Grid> V3dViewer_grid(const OccHandle<V3d_Viewer>& viewer);
    static AspectGridColors V3dViewer_gridColors(const OccHandle<V3d_Viewer>& viewer);
    static void V3dViewer_setGridColors(const OccHandle<V3d_Viewer>& viewer, const AspectGridColors& colors);

    static void AisContext_eraseObject(
        const OccHandle<AIS_InteractiveContext>& context,
        const OccHandle<AIS_InteractiveObject>& object
    );
    static void AisContext_setObjectVisible(
        const OccHandle<AIS_InteractiveContext>& context,
        const OccHandle<AIS_InteractiveObject>& object,
        bool on
    );

    static AIS_InteractiveContext* AisObject_contextPtr(const OccHandle<AIS_InteractiveObject>& object);
    static bool AisObject_isVisible(const OccHandle<AIS_InteractiveObject>& object);
    static void AisObject_setVisible(const OccHandle<AIS_InteractiveObject>& object, bool on);
    static Bnd_Box AisObject_boundingBox(const OccHandle<AIS_InteractiveObject>& object);

    static int AspectWindow_width(const OccHandle<Aspect_Window>& wnd);
    static int AspectWindow_height(const OccHandle<Aspect_Window>& wnd);
    static OccHandle<Aspect_DisplayConnection> AspectDisplayConnection_create();

    static void Gfx3dClipPlane_setCappingHatch(
        const OccHandle<Graphic3d_ClipPlane>& plane, Aspect_HatchStyle hatch
    );
    static void Gfx3dClipPlane_setNormal(
        const OccHandle<Graphic3d_ClipPlane>& plane, const gp_Dir& n
    );
    static void Gfx3dClipPlane_setPosition(
        const OccHandle<Graphic3d_ClipPlane>& plane, double pos
    );

    static bool ImagePixmap_flipY(Image_PixMap& pixmap);

};

} // namespace Mayo
