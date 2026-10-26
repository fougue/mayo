/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Prs3d_LineAspect.hxx>
#include <TopExp_Explorer.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <XCAFPrs_AISObject.hxx>

namespace Mayo {

namespace {
struct GraphicsShapeObjectDriverI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsShapeObjectDriver) };

// A face without trimming wires may still have a finite natural domain (eg a
// sphere or a Bezier surface), or carry a mesh. Keep those faces displayable.
bool isUnboundedUnmeshedFace(const TopoDS_Face& face)
{
    if (TopExp_Explorer(face, TopAbs_WIRE).More() || BRep_Tool::Surface(face).IsNull())
        return false;

    TopLoc_Location location;
    const auto& triangulation = BRep_Tool::Triangulation(face, location);
    if (!triangulation.IsNull() && triangulation->NbTriangles() > 0)
        return false;

    double uMin, uMax, vMin, vMax;
    BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);
    return Precision::IsInfinite(uMin) || Precision::IsInfinite(uMax)
            || Precision::IsInfinite(vMin) || Precision::IsInfinite(vMax);
}

class XcafShapeObject : public XCAFPrs_AISObject {
public:
    explicit XcafShapeObject(const TDF_Label& label)
        : XCAFPrs_AISObject(label)
    {
    }

    void DispatchStyles(Standard_Boolean toSyncStyles = Standard_False) override
    {
        XCAFPrs_AISObject::DispatchStyles(toSyncStyles);
        m_hasUnboundedFaces = false;

        // OCCT's shaded fallback draws five U/V isocurves for an unmeshed
        // face. On an unbounded surface these spoil the presentation and its
        // fit-all bounds. Suppress only that face in the presentation, keeping
        // the XDE shape, its styles and export geometry intact. Apply this
        // after every style dispatch: the base implementation clears drawers.
        BRepUtils::forEachSubFace(this->Shape(), [this](const TopoDS_Face& face) {
            if (isUnboundedUnmeshedFace(face)) {
                this->CustomAspects(face)->SetHidden(true);
                m_hasUnboundedFaces = true;
            }
        });
    }

protected:
    void ComputeSelection(const OccHandle<SelectMgr_Selection>& selection, int mode) override
    {
        // Bounding-box measurement selects compounds. OCCT's compound mode
        // bypasses hidden child drawers when constructing sensitive geometry.
        // Mode 0 yields the same original whole-shape owner while respecting
        // those drawers. The measurement tool also enables compound mode on
        // shell/solid roots, which otherwise have no compound to select.
        if (m_hasUnboundedFaces && mode == AIS_Shape::SelectionMode(TopAbs_COMPOUND))
        {
            AIS_ColoredShape::ComputeSelection(selection, 0);
        }
        else {
            XCAFPrs_AISObject::ComputeSelection(selection, mode);
        }
    }

private:
    bool m_hasUnboundedFaces = false;
};
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
        auto object = new XcafShapeObject(label);
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

    AIS_InteractiveContext* context = object ? object->InteractiveContext() : nullptr;
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
    if (object->InteractiveContext()->DrawHiddenLine())
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
GraphicsShapeObjectDriver::properties(gsl::span<const GraphicsObjectPtr> spanObject) const
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
