/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_shape_object_driver.h"

#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/triangulation_annex_data.h"
#include "../base/label_data.h"
#include "../base/xcaf.h"
#include "graphics_utils.h"

#include <AIS_ConnectedInteractive.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_DisplayMode.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <XCAFPrs_AISObject.hxx>

namespace Mayo {

namespace {
struct GraphicsShapeObjectDriverI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsShapeObjectDriver) };
} // namespace

GraphicsShapeObjectDriver::GraphicsShapeObjectDriver()
{
    this->setDisplayModes({
        { DisplayMode_Wireframe, GraphicsShapeObjectDriverI18N::textId("Wireframe") },
        { DisplayMode_HiddenLineRemoval, GraphicsShapeObjectDriverI18N::textId("HiddenLineRemoval") },
        { DisplayMode_Shaded, GraphicsShapeObjectDriverI18N::textId("Shaded") },
        { DisplayMode_ShadedWithFaceBoundary, GraphicsShapeObjectDriverI18N::textId("ShadedWithFaceBoundary") }
    });
    this->setDefaultDisplayMode(DisplayMode_ShadedWithFaceBoundary);
}

GraphicsObjectDriver::Support GraphicsShapeObjectDriver::supportStatus(const TDF_Label& label) const
{
    return shapeSupportStatus(label);
}

GraphicsObjectPtr GraphicsShapeObjectDriver::createObject(const TDF_Label& label) const
{
    if (XCaf::isShape(label)) {
        auto object = new XCAFPrs_AISObject(label);
        object->SetDisplayMode(AIS_Shaded);
        object->SetMaterial(Graphic3d_NOM_PLASTER);
        object->Attributes()->SetFaceBoundaryDraw(true);
        object->Attributes()->SetFaceBoundaryAspect(
            new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.)
        );
        object->Attributes()->SetAutoTriangulation(false);
        object->Attributes()->SetIsoOnTriangulation(true);
        //object->Attributes()->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr, true/*overrideDefaults*/);
        object->SetOwner(this);
        return object;
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
            auto aisLink = OccHandle<AIS_ConnectedInteractive>::DownCast(object);
            if (aisLink && aisLink->HasConnection()) {
                aisLink->ConnectedTo()->Attributes()->SetFaceBoundaryDraw(showFaceBounds);
                aisLink->ConnectedTo()->Redisplay(true);
            }
            else {
                object->Redisplay(true/*AllModes*/);
            }
        }
    }
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

std::unique_ptr<PropertyGroup>
GraphicsShapeObjectDriver::properties(Span<const GraphicsObjectPtr> spanObject) const
{
    this->throwIf_differentDriver(spanObject);
    //return std::make_unique<GraphicsObjectBasePropertyGroup>(spanObject);
    return {};
}

GraphicsObjectDriver::Support GraphicsShapeObjectDriver::shapeSupportStatus(const TDF_Label& label)
{
    const LabelDataFlags flags = findLabelDataFlags(label);
    if (flags & LabelData_ShapeIsFace) {
        if (flags & LabelData_ShapeIsGeometricFace)
            return GraphicsObjectDriver::Support::Complete;
        else if (flags & LabelData_HasTriangulationAnnexData)
            return GraphicsObjectDriver::Support::Partial;
    }

    if (flags & LabelData_HasShape)
        return GraphicsObjectDriver::Support::Complete;

    return GraphicsObjectDriver::Support::None;
    // TODO TNaming_Shape? TDataXtd_Shape?
}

} // namespace Mayo
