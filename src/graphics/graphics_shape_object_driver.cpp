/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_shape_object_driver.h"

#include "../base/caf_utils.h"
#include "../base/data_triangulation.h"
#include "../base/xcaf.h"
#include "graphics_utils.h"

#include <AIS_ConnectedInteractive.hxx>
#include <AIS_DisplayMode.hxx>
#include <XCAFPrs_AISObject.hxx>

namespace Mayo {

namespace {
struct GraphicsShapeObjectDriverI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsShapeObjectDriver) };
} // namespace

GraphicsShapeObjectDriver::GraphicsShapeObjectDriver()
{
    this->setDisplayModes({
        { DisplayMode_Wireframe, GraphicsShapeObjectDriverI18N::textId("Shape_Wireframe") },
        { DisplayMode_HiddenLineRemoval, GraphicsShapeObjectDriverI18N::textId("Shape_HiddenLineRemoval") },
        { DisplayMode_Shaded, GraphicsShapeObjectDriverI18N::textId("Shape_Shaded") },
        { DisplayMode_ShadedWithFaceBoundary, GraphicsShapeObjectDriverI18N::textId("Shape_ShadedWithFaceBoundary") }
    });
    this->setDefaultDisplayMode(DisplayMode_ShadedWithFaceBoundary);
}

GraphicsObjectDriver::Support GraphicsShapeObjectDriver::supportStatus(const TDF_Label& label) const
{
    if (XCaf::isShape(label))
        return Support::Complete;

    // TODO TNaming_Shape ?
    // TDataXtd_Shape ?

    if (CafUtils::hasAttribute<DataTriangulation>(label)) {
        //return Support::Partial;
    }

    return Support::None;
}

GraphicsObjectPtr GraphicsShapeObjectDriver::createObject(const TDF_Label& label) const
{
    if (XCaf::isShape(label)) {
        auto object = new XCAFPrs_AISObject(label);
        object->SetDisplayMode(AIS_Shaded);
        object->SetMaterial(Graphic3d_NOM_PLASTER);
        object->Attributes()->SetFaceBoundaryDraw(true);
        object->Attributes()->SetFaceBoundaryAspect(
                    new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.));
        object->Attributes()->SetIsoOnTriangulation(true);
        //object->Attributes()->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr, true/*overrideDefaults*/);
        object->SetOwner(this);
        return object;
    }
    else if (CafUtils::hasAttribute<DataTriangulation>(label)) {
    }

    return {};
}

void GraphicsShapeObjectDriver::applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const
{
    this->throwIf_differentDriver(object);
    this->throwIf_invalidDisplayMode(mode);

    if (mode == this->currentDisplayMode(object))
        return;

    AIS_InteractiveContext* context = GraphicsUtils::AisObject_contextPtr(object);
    if (!context)
        return;

    auto fnSetViewComputedMode = [=](bool on) {
        for (auto it = context->CurrentViewer()->DefinedViewIterator(); it.More(); it.Next())
            it.Value()->SetComputedMode(on);
    };

    if (mode == DisplayMode_HiddenLineRemoval) {
        context->DefaultDrawer()->SetTypeOfHLR(Prs3d_TOH_PolyAlgo);
        context->DefaultDrawer()->EnableDrawHiddenLine();
        fnSetViewComputedMode(true);
    }
    else {
        context->DefaultDrawer()->SetTypeOfHLR(Prs3d_TOH_NotSet);
        context->DefaultDrawer()->DisableDrawHiddenLine();
        fnSetViewComputedMode(false);
        const AIS_DisplayMode aisDispMode = mode == DisplayMode_Wireframe ? AIS_WireFrame : AIS_Shaded;
        const bool showFaceBounds = mode == DisplayMode_ShadedWithFaceBoundary;
        if (object->DisplayMode() != aisDispMode)
            context->SetDisplayMode(object, aisDispMode, false);

        if (object->Attributes()->FaceBoundaryDraw() != showFaceBounds) {
            object->Attributes()->SetFaceBoundaryDraw(showFaceBounds);
            auto aisLink = Handle_AIS_ConnectedInteractive::DownCast(object);
            if (aisLink && aisLink->HasConnection()) {
                aisLink->ConnectedTo()->Attributes()->SetFaceBoundaryDraw(showFaceBounds);
                aisLink->ConnectedTo()->Redisplay(true);
            }
            else {
                object->Redisplay(true);
            }
        }
    }

    // context->UpdateCurrentViewer();
}

Enumeration::Value GraphicsShapeObjectDriver::currentDisplayMode(const GraphicsObjectPtr& object) const
{
    this->throwIf_differentDriver(object);
    if (GraphicsUtils::AisObject_contextPtr(object)->DrawHiddenLine())
        return DisplayMode_HiddenLineRemoval;

    const int displayMode = object->DisplayMode();
    if (displayMode == AIS_WireFrame)
        return DisplayMode_Wireframe;

    if (displayMode == AIS_Shaded) {
        return object->Attributes()->FaceBoundaryDraw() ?
                    DisplayMode_ShadedWithFaceBoundary :
                    DisplayMode_Shaded;
    }

    return -1;
}

std::unique_ptr<GraphicsObjectBasePropertyGroup>
GraphicsShapeObjectDriver::properties(Span<const GraphicsObjectPtr> spanObject) const
{
    this->throwIf_differentDriver(spanObject);
    //return std::make_unique<GraphicsObjectBasePropertyGroup>(spanObject);
    return {};
}

} // namespace Mayo
