#include "graphics_entity_driver.h"

#include "../base/document.h"
#include "../base/caf_utils.h"

#include <BRep_TFace.hxx>
#include <MeshVS_DisplayModeFlags.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <XSDRAWSTLVRML_DataSource.hxx>
#include <stdexcept>

namespace Mayo {

void GraphicsEntityDriver::throwIf_invalidDisplayMode(Enumeration::Value mode) const
{
    if (this->displayModes().findIndex(mode) == -1)
        throw std::invalid_argument("Invalid display mode");
}

void GraphicsEntityDriver::initEntity(GraphicsEntity* ptrEntity, const TDF_Label& label) const
{
    Expects(ptrEntity != nullptr);
    ptrEntity->m_label = label;
    ptrEntity->m_driverPtr = this;
}

void GraphicsEntityDriver::setEntityAisObject(
        GraphicsEntity* ptrEntity, const Handle_AIS_InteractiveObject& obj)
{
    Expects(ptrEntity != nullptr);
    ptrEntity->m_aisObject = obj;
}

GraphicsShapeEntityDriver::GraphicsShapeEntityDriver()
{
    this->setDisplayModes({
        { DisplayMode_Wireframe, "WIREFRAME", tr("Wireframe") },
        { DisplayMode_Shaded, "SHADED", tr("Shaded") },
        { DisplayMode_ShadedWithFaceBoundary, "SHADED_FACE_BNDS", tr("Shaded with face boundaries") }
    });
}

GraphicsEntityDriver::Support GraphicsShapeEntityDriver::supportStatus(const TDF_Label& label) const
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

GraphicsEntity GraphicsShapeEntityDriver::createEntity(const TDF_Label& label) const
{
    GraphicsEntity entity;
    this->initEntity(&entity, label);
    if (XCaf::isShape(label)) {
        Handle_XCAFPrs_AISObject gpx = new XCAFPrs_AISObject(label);
        gpx->SetDisplayMode(AIS_Shaded);
        gpx->Attributes()->SetFaceBoundaryDraw(true);
        gpx->Attributes()->SetIsoOnTriangulation(true);
        GraphicsEntityDriver::setEntityAisObject(&entity, gpx);
    }
    else if (CafUtils::hasAttribute<TDataXtd_Triangulation>(label)) {
    }

    return entity;
}

void GraphicsShapeEntityDriver::applyDisplayMode(const GraphicsEntity& entity, Enumeration::Value mode) const
{
    this->throwIf_invalidDisplayMode(mode);
    const AIS_DisplayMode aisDispMode = mode == DisplayMode_Wireframe ? AIS_WireFrame : AIS_Shaded;
    const bool showFaceBounds = mode == DisplayMode_ShadedWithFaceBoundary;
    const Handle_AIS_InteractiveObject& aisObject = entity.aisObject();
    if (aisObject->DisplayMode() != aisDispMode)
        entity.aisContextPtr()->SetDisplayMode(aisObject, aisDispMode, false);

    if (aisObject->Attributes()->FaceBoundaryDraw() != showFaceBounds) {
        aisObject->Attributes()->SetFaceBoundaryDraw(showFaceBounds);
        aisObject->Redisplay(true);
    }

    //entity->aisContext()->UpdateCurrentViewer();
}

GraphicsMeshEntityDriver::GraphicsMeshEntityDriver()
{
    this->setDisplayModes({
        { MeshVS_DMF_WireFrame, "WIREFRAME", tr("Wireframe") },
        { MeshVS_DMF_Shading, "SHADED", tr("Shaded") },
        { MeshVS_DMF_Shrink, "SHRINK", tr("Shrink") } // MeshVS_DA_ShrinkCoeff
    });
}

GraphicsEntityDriver::Support GraphicsMeshEntityDriver::supportStatus(const TDF_Label& label) const
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

GraphicsEntity GraphicsMeshEntityDriver::createEntity(const TDF_Label& label) const
{
    GraphicsEntity entity;
    this->initEntity(&entity, label);
    Handle_Poly_Triangulation polyTri;
    const TopLoc_Location* ptrLocationPolyTri = nullptr;
    auto attrTriangulation = CafUtils::findAttribute<TDataXtd_Triangulation>(label);
    if (!attrTriangulation.IsNull()) {
        polyTri = attrTriangulation->Get();
    }
    else if (XCaf::isShape(label)) {
        const TopoDS_Shape shape = XCaf::shape(label);
        if (shape.ShapeType() == TopAbs_FACE) {
            const auto& tface = Handle_BRep_TFace::DownCast(shape.TShape());
            if (!tface.IsNull()) {
                polyTri = tface->Triangulation();
                ptrLocationPolyTri = &shape.Location();
            }
        }
    }

    if (!polyTri.IsNull()) {
        Handle_XSDRAWSTLVRML_DataSource dataSource = new XSDRAWSTLVRML_DataSource(polyTri);
        Handle_MeshVS_Mesh gpx = new MeshVS_Mesh;
        gpx->SetDataSource(dataSource);
        // meshVisu->AddBuilder(..., false); -> No selection
        gpx->AddBuilder(new MeshVS_MeshPrsBuilder(gpx), true);
        gpx->SetHilightMode(MeshVS_DMF_WireFrame);
        gpx->SetMeshSelMethod(MeshVS_MSM_PRECISE);
        GraphicsEntityDriver::setEntityAisObject(&entity, gpx);
    }

    return entity;
}

void GraphicsMeshEntityDriver::applyDisplayMode(const GraphicsEntity& entity, Enumeration::Value mode) const
{
    this->throwIf_invalidDisplayMode(mode);
    entity.aisContextPtr()->SetDisplayMode(entity.aisObject(), mode, false);
    //entity->aisContext()->UpdateCurrentViewer();
}

} // namespace Mayo
