#include "gpx_brep_shape_item.h"

#include "options.h"
#include "fougtools/occtools/qt_utils.h"

#include <AIS_InteractiveContext.hxx>

namespace Mayo {

GpxBRepShapeItem::GpxBRepShapeItem(BRepShapeItem *item)
    : GpxCovariantDocumentItem(item),
      propertyTransparency(this, tr("Transparency"), 0, 100, 5),
      propertyDisplayMode(this, tr("Display mode"), &enum_DisplayMode()),
      propertyShowFaceBoundary(this, tr("Show face boundary"))
{
    const Options* opts = Options::instance();

    // Create the AIS_Shape object
    Handle_AIS_Shape aisShape = new AIS_Shape(item->brepShape());
    aisShape->SetMaterial(opts->brepShapeDefaultMaterial());
    aisShape->SetDisplayMode(AIS_Shaded);
    aisShape->SetColor(occ::QtUtils::toOccColor(opts->brepShapeDefaultColor()));
    aisShape->Attributes()->SetFaceBoundaryDraw(Standard_True);
    aisShape->Attributes()->SetIsoOnTriangulation(Standard_True);

    m_hndGpxObject = aisShape;

    // Init properties
    Mayo_PropertyChangedBlocker(this);
    this->propertyMaterial.setValue(opts->brepShapeDefaultMaterial());
    this->propertyColor.setValue(m_hndGpxObject->Color());
    this->propertyTransparency.setValue(
                static_cast<int>(m_hndGpxObject->Transparency() * 100));
    this->propertyDisplayMode.setValue(m_hndGpxObject->DisplayMode());
    this->propertyShowFaceBoundary.setValue(
                m_hndGpxObject->Attributes()->FaceBoundaryDraw() == Standard_True);
}

void GpxBRepShapeItem::onPropertyChanged(Property *prop)
{
    Handle_AIS_InteractiveContext cxt = this->gpxObject()->GetContext();
    const Handle_AIS_InteractiveObject& hndGpx = this->handleGpxObject();
    AIS_Shape* ptrGpx = this->gpxObject();
    if (prop == &this->propertyMaterial) {
        ptrGpx->SetMaterial(
                    this->propertyMaterial.valueAs<Graphic3d_NameOfMaterial>());
        cxt->UpdateCurrentViewer();
    }
    else if (prop == &this->propertyColor) {
        ptrGpx->SetColor(this->propertyColor.value());
    }
    else if (prop == &this->propertyTransparency) {
        cxt->SetTransparency(hndGpx, this->propertyTransparency.value() / 100.);
    }
    else if (prop == &this->propertyDisplayMode) {
        cxt->SetDisplayMode(hndGpx, this->propertyDisplayMode.value());
    }
    else if (prop == &this->propertyShowFaceBoundary) {
        ptrGpx->Attributes()->SetFaceBoundaryDraw(
                    this->propertyShowFaceBoundary.value());
        ptrGpx->Redisplay(Standard_True); // All modes
        cxt->UpdateCurrentViewer();
    }
    GpxDocumentItem::onPropertyChanged(prop);
}

const Enumeration &GpxBRepShapeItem::enum_DisplayMode()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.map(AIS_Shaded, tr("Shaded"));
        enumeration.map(AIS_WireFrame, tr("Wireframe"));
    }
    return enumeration;
}

} // namespace Mayo
