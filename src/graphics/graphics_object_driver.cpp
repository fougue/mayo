/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_object_driver.h"

#include "../base/document.h"
#include "../base/caf_utils.h"
#include "../base/property_enumeration.h"
#include "graphics_object_base_property_group.h"
#include "graphics_mesh_data_source.h"
#include "graphics_scene.h"
#include "graphics_utils.h"

#include <AIS_ConnectedInteractive.hxx>
#include <AIS_DisplayMode.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRep_TFace.hxx>
#include <MeshVS_DisplayModeFlags.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <stdexcept>

namespace Mayo {

namespace { struct GraphicsObjectDriverI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsObjectDriver) }; }

GraphicsObjectDriverPtr GraphicsObjectDriver::get(const GraphicsObjectPtr& object)
{
    if (object)
        return GraphicsObjectDriverPtr::DownCast(object->GetOwner());
    else
        return {};
}

GraphicsObjectDriverPtr GraphicsObjectDriver::getCommon(Span<const GraphicsObjectPtr> spanObject)
{
    GraphicsObjectDriverPtr commonGfxDriver;
    for (const GraphicsObjectPtr& object : spanObject) {
        GraphicsObjectDriverPtr gfxDriver = GraphicsObjectDriver::get(object);
        if (!commonGfxDriver)
            commonGfxDriver = gfxDriver;
        else if (commonGfxDriver != gfxDriver)
            return {};
    }

    return commonGfxDriver;
}

void GraphicsObjectDriver::throwIf_invalidDisplayMode(Enumeration::Value mode) const
{
    if (this->displayModes().findIndex(mode) == -1)
        throw std::invalid_argument("Invalid display mode");
}

void GraphicsObjectDriver::throwIf_differentDriver(const GraphicsObjectPtr& object) const
{
    if (GraphicsObjectDriver::get(object) != this)
        throw std::invalid_argument("Invalid driver for graphics object");
}

void GraphicsObjectDriver::throwIf_differentDriver(Span<const GraphicsObjectPtr> objects) const
{
    for (const GraphicsObjectPtr& object : objects)
        this->throwIf_differentDriver(object);
}

GraphicsShapeObjectDriver::GraphicsShapeObjectDriver()
{
    this->setDisplayModes({
        { DisplayMode_Wireframe, GraphicsObjectDriverI18N::textId("Shape_Wireframe") },
        { DisplayMode_HiddenLineRemoval, GraphicsObjectDriverI18N::textId("Shape_HiddenLineRemoval") },
        { DisplayMode_Shaded, GraphicsObjectDriverI18N::textId("Shape_Shaded") },
        { DisplayMode_ShadedWithFaceBoundary, GraphicsObjectDriverI18N::textId("Shape_ShadedWithFaceBoundary") }
    });
    this->setDefaultDisplayMode(DisplayMode_ShadedWithFaceBoundary);
}

GraphicsObjectDriver::Support GraphicsShapeObjectDriver::supportStatus(const TDF_Label& label) const
{
    if (XCaf::isShape(label))
        return Support::Complete;

    // TODO TNaming_Shape ?
    // TDataXtd_Shape ?

    if (CafUtils::hasAttribute<TDataXtd_Triangulation>(label)) {
        //return Support::Partial;
    }

    return Support::None;
}

GraphicsObjectPtr GraphicsShapeObjectDriver::createObject(const TDF_Label& label) const
{
    if (XCaf::isShape(label)) {
//        Handle_AIS_Shape object = new AIS_Shape(XCaf::shape(label));
        Handle_XCAFPrs_AISObject object = new XCAFPrs_AISObject(label);
        object->SetDisplayMode(AIS_Shaded);
        object->Attributes()->SetFaceBoundaryDraw(true);
        object->Attributes()->SetFaceBoundaryAspect(
                    new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.));
        object->Attributes()->SetIsoOnTriangulation(true);
        object->SetOwner(this);
        return object;
    }
    else if (CafUtils::hasAttribute<TDataXtd_Triangulation>(label)) {
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
        V3d_ListOfViewIterator viewIter = context->CurrentViewer()->DefinedViewIterator();
        while (viewIter.More()) {
            viewIter.Value()->SetComputedMode(on);
            viewIter.Next();
        }
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

GraphicsMeshObjectDriver::GraphicsMeshObjectDriver()
{
    this->setDisplayModes({
        { MeshVS_DMF_WireFrame, GraphicsObjectDriverI18N::textId("Mesh_Wireframe") },
        { MeshVS_DMF_Shading, GraphicsObjectDriverI18N::textId("Mesh_Shaded") },
        { MeshVS_DMF_Shrink, GraphicsObjectDriverI18N::textId("Mesh_Shrink") } // MeshVS_DA_ShrinkCoeff
    });
    this->setDefaultDisplayMode(MeshVS_DMF_Shading);
}

GraphicsObjectDriver::Support GraphicsMeshObjectDriver::supportStatus(const TDF_Label& label) const
{
    if (CafUtils::hasAttribute<TDataXtd_Triangulation>(label))
        return Support::Complete;

    if (XCaf::isShape(label)) {
        const TopoDS_Shape shape = XCaf::shape(label);
        if (shape.ShapeType() == TopAbs_FACE)
            return Support::Partial;
    }

    return Support::None;
}

GraphicsObjectPtr GraphicsMeshObjectDriver::createObject(const TDF_Label& label) const
{
    Handle_Poly_Triangulation polyTri;
    //const TopLoc_Location* ptrLocationPolyTri = nullptr;
    auto attrTriangulation = CafUtils::findAttribute<TDataXtd_Triangulation>(label);
    if (attrTriangulation) {
        polyTri = attrTriangulation->Get();
    }
    else if (XCaf::isShape(label)) {
        const TopoDS_Shape shape = XCaf::shape(label);
        if (shape.ShapeType() == TopAbs_FACE) {
            auto tface = Handle_BRep_TFace::DownCast(shape.TShape());
            if (tface) {
                polyTri = tface->Triangulation();
                //ptrLocationPolyTri = &shape.Location();
            }
        }
    }

    if (polyTri) {
        Handle_MeshVS_Mesh object = new MeshVS_Mesh;
        object->SetDataSource(new GraphicsMeshDataSource(polyTri));
        // meshVisu->AddBuilder(..., false); -> No selection
        object->AddBuilder(new MeshVS_MeshPrsBuilder(object), true);

        // -- MeshVS_DrawerAttribute
        object->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, defaultValues().showEdges);
        object->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, defaultValues().showNodes);
        object->GetDrawer()->SetMaterial(MeshVS_DA_FrontMaterial, Graphic3d_NOM_PLASTIC);
        object->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, defaultValues().color);
        object->GetDrawer()->SetMaterial(
                    MeshVS_DA_FrontMaterial, Graphic3d_MaterialAspect(defaultValues().material));
        object->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, defaultValues().edgeColor);
        object->SetDisplayMode(MeshVS_DMF_Shading);

        //object->SetHilightMode(MeshVS_DMF_WireFrame);
        object->SetMeshSelMethod(MeshVS_MSM_PRECISE);

        object->SetOwner(this);
        return object;
    }

    return {};
}

void GraphicsMeshObjectDriver::applyDisplayMode(GraphicsObjectPtr object, Enumeration::Value mode) const
{
    this->throwIf_differentDriver(object);
    this->throwIf_invalidDisplayMode(mode);
    GraphicsUtils::AisObject_contextPtr(object)->SetDisplayMode(object, mode, false);
}

Enumeration::Value GraphicsMeshObjectDriver::currentDisplayMode(const GraphicsObjectPtr& object) const
{
    this->throwIf_differentDriver(object);
    return object->DisplayMode();
}

class GraphicsMeshObjectDriver::ObjectProperties : public GraphicsObjectBasePropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsMeshObjectDriver_ObjectProperties)
public:
    ObjectProperties(Span<const GraphicsObjectPtr> spanObject)
        : GraphicsObjectBasePropertyGroup(spanObject)
    {
        NCollection_Vec3<float> sumColor = {};
        NCollection_Vec3<float> sumEdgeColor = {};
        int countShowEdges = 0;
        int countShowNodes = 0;
        for (const GraphicsObjectPtr& object : spanObject) {
            auto meshVisu = Handle_MeshVS_Mesh::DownCast(object);
            // Color
            Quantity_Color color;
            meshVisu->GetDrawer()->GetColor(MeshVS_DA_InteriorColor, color);
            sumColor += color;
            // Edge color
            meshVisu->GetDrawer()->GetColor(MeshVS_DA_EdgeColor, color);
            sumEdgeColor += color;
            // Show edges
            bool boolVal;
            meshVisu->GetDrawer()->GetBoolean(MeshVS_DA_ShowEdges, boolVal);
            countShowEdges += boolVal ? 1 : 0;
            // Show nodes
            meshVisu->GetDrawer()->GetBoolean(MeshVS_DA_DisplayNodes, boolVal);
            countShowNodes += boolVal ? 1 : 0;

            m_vecMeshVisu.push_back(meshVisu);
        }

        auto fnCheckState = [&](int count) {
            if (count == 0)
                return Qt::Unchecked;
            else
                return count == spanObject.size() ? Qt::Checked : Qt::PartiallyChecked;
        };

        // Init properties
        Mayo_PropertyChangedBlocker(this);

        m_propertyColor.setValue(Quantity_Color(sumColor / float(spanObject.size())));
        m_propertyEdgeColor.setValue(Quantity_Color(sumEdgeColor / float(spanObject.size())));
        m_propertyShowEdges.setValue(fnCheckState(countShowEdges));
        m_propertyShowNodes.setValue(fnCheckState(countShowNodes));
    }

    void onPropertyChanged(Property* prop) override {
        auto fnRedisplay = [](const GraphicsObjectPtr& object) {
            object->Redisplay(true); // All modes
        };

        if (prop == &m_propertyShowEdges) {
            if (m_propertyShowEdges != Qt::PartiallyChecked) {
                for (const Handle_MeshVS_Mesh& meshVisu : m_vecMeshVisu) {
                    meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, m_propertyShowEdges == Qt::Checked);
                    fnRedisplay(meshVisu);
                }
            }
        }
        else if (prop == &m_propertyShowNodes) {
            if (m_propertyShowNodes != Qt::PartiallyChecked) {
                for (const Handle_MeshVS_Mesh& meshVisu : m_vecMeshVisu) {
                    meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, m_propertyShowNodes == Qt::Checked);
                    fnRedisplay(meshVisu);
                }
            }
        }
        else if (prop == &m_propertyColor) {
            for (const Handle_MeshVS_Mesh& meshVisu : m_vecMeshVisu) {
                meshVisu->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, m_propertyColor);
                fnRedisplay(meshVisu);
            }
        }
        else if (prop == &m_propertyEdgeColor) {
            for (const Handle_MeshVS_Mesh& meshVisu : m_vecMeshVisu) {
                meshVisu->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, m_propertyEdgeColor);
                fnRedisplay(meshVisu);
            }
        }

        GraphicsObjectBasePropertyGroup::onPropertyChanged(prop);
    }

    std::vector<Handle_MeshVS_Mesh> m_vecMeshVisu;
    PropertyOccColor m_propertyColor{ this, textId("color") };
    PropertyOccColor m_propertyEdgeColor{ this, textId("edgeColor") };
    PropertyCheckState m_propertyShowEdges{ this, textId("showEdges") };
    PropertyCheckState m_propertyShowNodes{ this, textId("showNodes") };
};

std::unique_ptr<GraphicsObjectBasePropertyGroup>
GraphicsMeshObjectDriver::properties(Span<const GraphicsObjectPtr> spanObject) const
{
    this->throwIf_differentDriver(spanObject);
    return std::make_unique<ObjectProperties>(spanObject);
}

namespace Internal {

Q_GLOBAL_STATIC(GraphicsMeshObjectDriver::DefaultValues, graphicsMeshDefaultValues)

} // namespace Internal

const GraphicsMeshObjectDriver::DefaultValues& GraphicsMeshObjectDriver::defaultValues() {
    return *Internal::graphicsMeshDefaultValues;
}

void GraphicsMeshObjectDriver::setDefaultValues(const DefaultValues& values) {
    *Internal::graphicsMeshDefaultValues = values;
}

} // namespace Mayo
