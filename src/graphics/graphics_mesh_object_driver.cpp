/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_mesh_object_driver.h"

#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/label_data.h"
#include "../base/triangulation_annex_data.h"
#include "../base/property_builtins.h"
#include "../base/xcaf.h"
#include "graphics_mesh_data_source.h"
#include "graphics_utils.h"

#include <AIS_InteractiveContext.hxx>
#include <BRep_TFace.hxx>
#include <MeshVS_DisplayModeFlags.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

namespace Mayo {

namespace {
struct GraphicsMeshObjectDriverI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::GraphicsMeshObjectDriver) };
} // namespace

GraphicsMeshObjectDriver::GraphicsMeshObjectDriver()
{
    this->setDisplayModes({
        { DisplayMode_Wireframe, GraphicsMeshObjectDriverI18N::textId("Wireframe") },
        { DisplayMode_Shaded, GraphicsMeshObjectDriverI18N::textId("Shaded") },
        { DisplayMode_Shrink, GraphicsMeshObjectDriverI18N::textId("Shrink") } // MeshVS_DA_ShrinkCoeff
    });
    this->setDefaultDisplayMode(DisplayMode_Shaded);
}

GraphicsMeshObjectDriver::Support GraphicsMeshObjectDriver::supportStatus(const TDF_Label& label) const
{
    return meshSupportStatus(label);
}

GraphicsObjectPtr GraphicsMeshObjectDriver::createObject(const TDF_Label& label) const
{
    OccHandle<Poly_Triangulation> polyTri;
    Span<const Quantity_Color> spanNodeColor;
    //const TopLoc_Location* ptrLocationPolyTri = nullptr;
    if (XCaf::isShape(label)) {
        const TopoDS_Shape shape = XCaf::shape(label);
        if (shape.ShapeType() == TopAbs_FACE) {
            auto tface = OccHandle<BRep_TFace>::DownCast(shape.TShape());
            if (tface) {
                polyTri = tface->Triangulation();
                //ptrLocationPolyTri = &shape.Location();
            }

            auto attrMeshData = CafUtils::findAttribute<TriangulationAnnexData>(label);
            if (attrMeshData)
                spanNodeColor = attrMeshData->nodeColors();
        }
    }

    if (polyTri) {
        auto  object = makeOccHandle<MeshVS_Mesh>();
        object->SetDataSource(new GraphicsMeshDataSource(polyTri));
        // meshVisu->AddBuilder(..., false); -> No selection
        if (!spanNodeColor.empty()) {
            auto meshPrsBuilder = new MeshVS_NodalColorPrsBuilder(object, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
            for (int i = 0; CppUtils::cmpLess(i, spanNodeColor.size()); ++i)
                meshPrsBuilder->SetColor(i + 1, spanNodeColor[i]);

            object->AddBuilder(meshPrsBuilder, true);
        }
        else {
            object->AddBuilder(new MeshVS_MeshPrsBuilder(object), true);
        }

        // -- MeshVS_DrawerAttribute
        object->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, defaultValues().showEdges);
        object->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, defaultValues().showNodes);
        object->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, defaultValues().color);
        object->GetDrawer()->SetMaterial(
            MeshVS_DA_FrontMaterial, Graphic3d_MaterialAspect(defaultValues().material)
        );
        object->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, defaultValues().edgeColor);
        object->GetDrawer()->SetBoolean(MeshVS_DA_ColorReflection, true);
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

class GraphicsMeshObjectDriver::ObjectProperties : public PropertyGroup {
public:
    ObjectProperties(Span<const GraphicsObjectPtr> spanObject)
    {
        NCollection_Vec3<float> sumColor = {};
        NCollection_Vec3<float> sumEdgeColor = {};
        int countShowEdges = 0;
        int countShowNodes = 0;
        for (const GraphicsObjectPtr& object : spanObject) {
            auto meshVisu = OccHandle<MeshVS_Mesh>::DownCast(object);
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
                return CheckState::Off;
            else
                return CppUtils::cmpEqual(count, spanObject.size()) ? CheckState::On : CheckState::Partially;
        };

        // Init properties
        Mayo_PropertyChangedBlocker(this);

        m_propertyColor.setValue(Quantity_Color(sumColor / float(spanObject.size())));
        m_propertyEdgeColor.setValue(Quantity_Color(sumEdgeColor / float(spanObject.size())));
        m_propertyShowEdges.setValue(fnCheckState(countShowEdges));
        m_propertyShowNodes.setValue(fnCheckState(countShowNodes));
    }

    void onPropertyChanged(Property* prop) override
    {
        auto fnRedisplay = [](const GraphicsObjectPtr& object) {
            object->Redisplay(true); // All modes
        };

        if (prop == &m_propertyShowEdges) {
            if (m_propertyShowEdges.value() != CheckState::Partially) {
                for (const OccHandle<MeshVS_Mesh>& meshVisu : m_vecMeshVisu) {
                    meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, m_propertyShowEdges.value() == CheckState::On);
                    fnRedisplay(meshVisu);
                }
            }
        }
        else if (prop == &m_propertyShowNodes) {
            if (m_propertyShowNodes.value() != CheckState::Partially) {
                for (const OccHandle<MeshVS_Mesh>& meshVisu : m_vecMeshVisu) {
                    meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, m_propertyShowNodes.value() == CheckState::On);
                    fnRedisplay(meshVisu);
                }
            }
        }
        else if (prop == &m_propertyColor) {
            for (const OccHandle<MeshVS_Mesh>& meshVisu : m_vecMeshVisu) {
                meshVisu->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, m_propertyColor);
                fnRedisplay(meshVisu);
            }
        }
        else if (prop == &m_propertyEdgeColor) {
            for (const OccHandle<MeshVS_Mesh>& meshVisu : m_vecMeshVisu) {
                meshVisu->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, m_propertyEdgeColor);
                fnRedisplay(meshVisu);
            }
        }

        PropertyGroup::onPropertyChanged(prop);
    }

    std::vector<OccHandle<MeshVS_Mesh>> m_vecMeshVisu;
    PropertyOccColor m_propertyColor{ this, GraphicsMeshObjectDriverI18N::textId("color") };
    PropertyOccColor m_propertyEdgeColor{ this, GraphicsMeshObjectDriverI18N::textId("edgeColor") };
    PropertyCheckState m_propertyShowEdges{ this, GraphicsMeshObjectDriverI18N::textId("showEdges") };
    PropertyCheckState m_propertyShowNodes{ this, GraphicsMeshObjectDriverI18N::textId("showNodes") };
};

std::unique_ptr<PropertyGroup>
GraphicsMeshObjectDriver::properties(Span<const GraphicsObjectPtr> spanObject) const
{
    this->throwIf_differentDriver(spanObject);
    return std::make_unique<ObjectProperties>(spanObject);
}

GraphicsMeshObjectDriver::Support GraphicsMeshObjectDriver::meshSupportStatus(const TDF_Label& label)
{
    const LabelDataFlags flags = findLabelDataFlags(label);
    if (flags & LabelData_ShapeIsFace) {
        if (flags & LabelData_HasTriangulationAnnexData)
            return GraphicsMeshObjectDriver::Support::Complete;
        else
            return GraphicsMeshObjectDriver::Support::Partial;
    }

    return GraphicsMeshObjectDriver::Support::None;
}

namespace Internal {

GraphicsMeshObjectDriver::DefaultValues& graphicsMeshDefaultValues()
{
    static GraphicsMeshObjectDriver::DefaultValues global;
    return global;
}

} // namespace Internal

const GraphicsMeshObjectDriver::DefaultValues& GraphicsMeshObjectDriver::defaultValues()
{
    return Internal::graphicsMeshDefaultValues();
}

void GraphicsMeshObjectDriver::setDefaultValues(const DefaultValues& values)
{
    Internal::graphicsMeshDefaultValues() = values;
}

} // namespace Mayo
