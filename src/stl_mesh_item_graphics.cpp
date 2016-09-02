#include "stl_mesh_item_graphics.h"

#include <AIS_InteractiveContext.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>

namespace Mayo {

namespace Internal {

static void redisplayAndUpdateViewer(AIS_InteractiveObject* ptrGpx)
{
    ptrGpx->Redisplay(Standard_True); // All modes
    ptrGpx->GetContext()->UpdateCurrentViewer();
}

} // namespace Internal

StlMeshItemGraphics::StlMeshItemGraphics(StlMeshItem *item)
    : CovariantDocumentItemGraphics(item),
      propertyDisplayMode(this, tr("Display mode"), &enum_DisplayMode()),
      propertyShowEdges(this, tr("Show edges")),
      propertyShowNodes(this, tr("Show nodes"))
{
    Mayo_PropertyChangedBlocker(this);
    // Material
    Graphic3d_MaterialAspect materialAspect;
    m_hndGpxObject->GetDrawer()->GetMaterial(
                MeshVS_DA_FrontMaterial, materialAspect);
    this->propertyMaterial.setValue(materialAspect.Name());
    // Color
    Quantity_Color color;
    m_hndGpxObject->GetDrawer()->GetColor(MeshVS_DA_InteriorColor, color);
    this->propertyColor.setValue(color);
    // Display mode
    this->propertyDisplayMode.setValue(m_hndGpxObject->DisplayMode());
    // Show edges
    Standard_Boolean boolVal;
    m_hndGpxObject->GetDrawer()->GetBoolean(MeshVS_DA_ShowEdges, boolVal);
    this->propertyShowEdges.setValue(boolVal == Standard_True);
    // Show nodes
    m_hndGpxObject->GetDrawer()->GetBoolean(MeshVS_DA_DisplayNodes, boolVal);
    this->propertyShowNodes.setValue(boolVal == Standard_True);
}

void StlMeshItemGraphics::onPropertyChanged(Property *prop)
{
    MeshVS_Mesh* ptrGpx = this->gpxObject();
    if (prop == &this->propertyMaterial) {
        const Graphic3d_NameOfMaterial mat =
                this->propertyMaterial.valueAs<Graphic3d_NameOfMaterial>();
        ptrGpx->GetDrawer()->SetMaterial(
                    MeshVS_DA_FrontMaterial, Graphic3d_MaterialAspect(mat));
        Internal::redisplayAndUpdateViewer(ptrGpx);
    }
    else if (prop == &this->propertyColor) {
        ptrGpx->GetDrawer()->SetColor(
                    MeshVS_DA_InteriorColor, this->propertyColor.value());
        Internal::redisplayAndUpdateViewer(ptrGpx);
    }
    else if (prop == &this->propertyDisplayMode) {
        ptrGpx->SetDisplayMode(this->propertyDisplayMode.value());
    }
    else if (prop == &this->propertyShowEdges) {
        ptrGpx->GetDrawer()->SetBoolean(
                    MeshVS_DA_ShowEdges, this->propertyShowEdges.value());
        Internal::redisplayAndUpdateViewer(ptrGpx);
    }
    else if (prop == &this->propertyShowNodes) {
        ptrGpx->GetDrawer()->SetBoolean(
                    MeshVS_DA_DisplayNodes, this->propertyShowNodes.value());
        Internal::redisplayAndUpdateViewer(ptrGpx);
    }
}

const Enumeration &StlMeshItemGraphics::enum_DisplayMode()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.map(MeshVS_DMF_WireFrame, tr("Wireframe"));
        enumeration.map(MeshVS_DMF_Shading, tr("Shaded"));
        enumeration.map(MeshVS_DMF_Shrink, tr("Shrink"));
    }
    return enumeration;
}

} // namespace Mayo
