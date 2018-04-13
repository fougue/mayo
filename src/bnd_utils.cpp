/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "bnd_utils.h"

namespace Mayo {

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
    for (PrsMgr_ModedPresentation& pres : obj->Presentations()) {
        if (pres.Mode() == obj->DisplayMode()) {
            const Handle_Prs3d_Presentation& pres3d = pres.Presentation()->Presentation();
            if (!pres3d->CStructure()->BoundingBox().IsValid())
                pres3d->CalculateBoundBox();
        }
    }

    obj->BoundingBox(box);
    return box;
}

} // namespace Mayo
