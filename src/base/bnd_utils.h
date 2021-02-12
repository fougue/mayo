/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <array>
#include <Bnd_Box.hxx>
#include <gp_Pnt.hxx>

namespace Mayo {

struct BndUtils {
    static void add(Bnd_Box* box, const Bnd_Box& other);
};

struct BndBoxCoords {
    double xmin;
    double ymin;
    double zmin;
    double xmax;
    double ymax;
    double zmax;

    gp_Pnt center() const;
    std::array<gp_Pnt, 8> vertices() const;
    static BndBoxCoords get(const Bnd_Box& box);
};

} // namespace Mayo
