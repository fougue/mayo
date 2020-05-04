/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#if 0
#include "gpx_mesh_item.h"
#include "gpx_utils.h"

#include <fougtools/occtools/qt_utils.h>
#include <AIS_InteractiveContext.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <XSDRAWSTLVRML_DataSource.hxx>

namespace Mayo {

namespace Internal {

Q_GLOBAL_STATIC(GpxMeshItem::DefaultValues, defaultValues)

static void redisplayAndUpdateViewer(const Handle_AIS_InteractiveObject& gpx)
{
    gpx->Redisplay(true); // All modes
    gpx->GetContext()->UpdateCurrentViewer();
}

} // namespace Internal

GpxMeshItem::GpxMeshItem(MeshItem *item)
    : propertyDisplayMode(this, tr("Display mode"), &enum_DisplayMode()),
      propertyShowEdges(this, tr("Show edges")),
      propertyShowNodes(this, tr("Show nodes")),
      m_meshItem(item)
{
    // Create the MeshVS_Mesh object
    Handle_XSDRAWSTLVRML_DataSource dataSource =
            new XSDRAWSTLVRML_DataSource(item->triangulation());
    Handle_MeshVS_Mesh meshVisu = new MeshVS_Mesh;
    meshVisu->SetDataSource(dataSource);
    // meshVisu->AddBuilder(..., false); -> No selection
    meshVisu->AddBuilder(new MeshVS_MeshPrsBuilder(meshVisu), true);
    // -- MeshVS_DrawerAttribute
    meshVisu->GetDrawer()->SetBoolean(
                MeshVS_DA_ShowEdges, GpxMeshItem::defaultValues().showEdges);
    meshVisu->GetDrawer()->SetBoolean(
                MeshVS_DA_DisplayNodes, GpxMeshItem::defaultValues().showNodes);
    meshVisu->GetDrawer()->SetMaterial(
                MeshVS_DA_FrontMaterial,
                Graphic3d_MaterialAspect(GpxMeshItem::defaultValues().material));
    meshVisu->GetDrawer()->SetColor(
                MeshVS_DA_InteriorColor,
                occ::QtUtils::toOccColor(GpxMeshItem::defaultValues().color));
    meshVisu->SetDisplayMode(MeshVS_DMF_Shading);
    // -- Wireframe as default highlight mode
    meshVisu->SetHilightMode(MeshVS_DMF_WireFrame);
    meshVisu->SetMeshSelMethod(MeshVS_MSM_PRECISE);

    m_meshVisu = meshVisu;

    // Init properties
    Mayo_PropertyChangedBlocker(this);
    // -- Material
    Graphic3d_MaterialAspect materialAspect;
    meshVisu->GetDrawer()->GetMaterial(MeshVS_DA_FrontMaterial, materialAspect);
    this->propertyMaterial.setValue(materialAspect.Name());
    // -- Color
    Quantity_Color color;
    meshVisu->GetDrawer()->GetColor(MeshVS_DA_InteriorColor, color);
    this->propertyColor.setValue(color);
    // -- Display mode
    this->propertyDisplayMode.setValue(meshVisu->DisplayMode());
    // -- Show edges
    bool boolVal;
    meshVisu->GetDrawer()->GetBoolean(MeshVS_DA_ShowEdges, boolVal);
    this->propertyShowEdges.setValue(boolVal);
    // -- Show nodes
    meshVisu->GetDrawer()->GetBoolean(MeshVS_DA_DisplayNodes, boolVal);
    this->propertyShowNodes.setValue(boolVal);
}

GpxMeshItem::~GpxMeshItem()
{
    GpxUtils::AisContext_eraseObject(this->context(), m_meshVisu);
}

MeshItem *GpxMeshItem::documentItem() const
{
    return m_meshItem;
}

void GpxMeshItem::setVisible(bool on)
{
    GpxDocumentItem::setVisible(on);
    GpxUtils::AisContext_setObjectVisible(this->context(), m_meshVisu, on);
}

void GpxMeshItem::activateSelection(int mode)
{
    this->context()->Activate(m_meshVisu, mode);
}

std::vector<Handle_SelectMgr_EntityOwner> GpxMeshItem::entityOwners(int mode) const
{
    std::vector<Handle_SelectMgr_EntityOwner> vec;
    GpxDocumentItem::getEntityOwners(this->context(), m_meshVisu, mode, &vec);
    return vec;
}

Bnd_Box GpxMeshItem::boundingBox() const
{
    return GpxUtils::AisObject_boundingBox(m_meshVisu);
}

const GpxMeshItem::DefaultValues& GpxMeshItem::defaultValues()
{
    return *Internal::defaultValues;
}

void GpxMeshItem::setDefaultValues(const DefaultValues& values)
{
    *Internal::defaultValues = values;
}

void GpxMeshItem::onPropertyChanged(Property* prop)
{
    if (prop == &this->propertyMaterial) {
        const auto mat = this->propertyMaterial.valueAs<Graphic3d_NameOfMaterial>();
        m_meshVisu->GetDrawer()->SetMaterial(MeshVS_DA_FrontMaterial, Graphic3d_MaterialAspect(mat));
        Internal::redisplayAndUpdateViewer(m_meshVisu);
    }
    else if (prop == &this->propertyColor) {
        m_meshVisu->GetDrawer()->SetColor(MeshVS_DA_InteriorColor, this->propertyColor.value());
        Internal::redisplayAndUpdateViewer(m_meshVisu);
    }
    else if (prop == &this->propertyDisplayMode) {
        this->context()->SetDisplayMode(m_meshVisu, this->propertyDisplayMode.value(), true);
        //ptrGpx->SetDisplayMode(this->propertyDisplayMode.value());
    }
    else if (prop == &this->propertyShowEdges) {
        m_meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, this->propertyShowEdges.value());
        Internal::redisplayAndUpdateViewer(m_meshVisu);
    }
    else if (prop == &this->propertyShowNodes) {
        m_meshVisu->GetDrawer()->SetBoolean(MeshVS_DA_DisplayNodes, this->propertyShowNodes.value());
        Internal::redisplayAndUpdateViewer(m_meshVisu);
    }

    GpxDocumentItem::onPropertyChanged(prop);
}

const Enumeration& GpxMeshItem::enum_DisplayMode()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.addItem(MeshVS_DMF_WireFrame, tr("Wireframe"));
        enumeration.addItem(MeshVS_DMF_Shading, tr("Shaded"));
        enumeration.addItem(MeshVS_DMF_Shrink, tr("Shrink"));
    }

    return enumeration;
}

} // namespace Mayo
#endif
