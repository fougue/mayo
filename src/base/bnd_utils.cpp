/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "bnd_utils.h"

#include <Standard_Version.hxx>

namespace Mayo {

gp_Pnt BndBoxCoords::center() const
{
    return {
      (this->xmax + this->xmin) / 2.,
      (this->ymax + this->ymin) / 2.,
      (this->zmax + this->zmin) / 2.
    };
}

std::array<gp_Pnt, 8> BndBoxCoords::vertices() const
{
    return {{
            { this->xmin, this->ymin, this->zmax },
            { this->xmax, this->ymin, this->zmax },
            { this->xmin, this->ymin, this->zmin },
            { this->xmax, this->ymin, this->zmin },
            { this->xmin, this->ymax, this->zmax },
            { this->xmax, this->ymax, this->zmax },
            { this->xmin, this->ymax, this->zmin },
            { this->xmax, this->ymax, this->zmin }
        }};
}

BndBoxCoords BndBoxCoords::get(const Bnd_Box &box)
{
    BndBoxCoords bbc = {};
    if (!box.IsVoid())
        box.Get(bbc.xmin, bbc.ymin, bbc.zmin, bbc.xmax, bbc.ymax, bbc.zmax);
    return bbc;
}

void BndUtils::add(Bnd_Box *box, const Bnd_Box &other)
{
    const auto bbc = BndBoxCoords::get(other);
    for (const gp_Pnt& pnt : bbc.vertices())
        box->Add(pnt);
}

Bnd_Box BndUtils::get(const Handle_AIS_InteractiveObject &obj)
{
    Bnd_Box box;
    if (obj.IsNull())
        return box;

    // Ensure bounding box is calculated
#if OCC_VERSION_HEX >= 0x070400
    for (Handle_PrsMgr_Presentation prs : obj->Presentations()) {
        if (prs->Mode() == obj->DisplayMode() && !prs->CStructure()->BoundingBox().IsValid())
            prs->CalculateBoundBox();
    }
#else
    for (PrsMgr_ModedPresentation& pres : obj->Presentations()) {
        if (pres.Mode() == obj->DisplayMode()) {
            const Handle_Prs3d_Presentation& pres3d = pres.Presentation()->Presentation();
            if (!pres3d->CStructure()->BoundingBox().IsValid())
                pres3d->CalculateBoundBox();
        }
    }
#endif


    obj->BoundingBox(box);
    return box;
}

} // namespace Mayo
