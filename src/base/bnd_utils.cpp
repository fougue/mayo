/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "bnd_utils.h"
#include "tkernel_utils.h"

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

gp_Pnt BndBoxCoords::minVertex() const
{
    return { this->xmin, this->ymin, this->zmin };
}

gp_Pnt BndBoxCoords::maxVertex() const
{
    return { this->xmax, this->ymax, this->zmax };
}

BndBoxCoords BndBoxCoords::get(const Bnd_Box& box)
{
    BndBoxCoords bbc = {};
    if (!box.IsVoid())
        box.Get(bbc.xmin, bbc.ymin, bbc.zmin, bbc.xmax, bbc.ymax, bbc.zmax);
    return bbc;
}

void BndUtils::add(Bnd_Box* box, const Bnd_Box& other)
{
    const auto bbc = BndBoxCoords::get(other);
    for (const gp_Pnt& pnt : bbc.vertices())
        box->Add(pnt);
}

bool BndUtils::isOpen(const Bnd_Box& bndBox)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    return bndBox.IsOpen();
#else
    return bndBox.IsOpenXmax()
            || bndBox.IsOpenXmin()
            || bndBox.IsOpenYmax()
            || bndBox.IsOpenYmin()
            || bndBox.IsOpenZmax()
            || bndBox.IsOpenZmin();
#endif
}

bool BndUtils::hasFinitePart(const Bnd_Box& bndBox)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    return bndBox.HasFinitePart();
#else
    const auto coords = BndBoxCoords::get(bndBox);
    return !bndBox.IsVoid() && coords.xmax >= coords.xmin;
#endif
}

Bnd_Box BndUtils::finitePart(const Bnd_Box& bndBox)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    return bndBox.FinitePart();
#else
    if (!BndUtils::hasFinitePart(bndBox))
        return Bnd_Box();

    const auto coords = BndBoxCoords::get(bndBox);
    Bnd_Box otherBox;
    otherBox.Update(coords.xmin, coords.ymin, coords.zmin, coords.xmax, coords.ymax, coords.zmax);
    otherBox.SetGap(bndBox.GetGap());
    return otherBox;
#endif
}

} // namespace Mayo
