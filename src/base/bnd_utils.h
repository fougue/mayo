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

// Provides helper functions for OpenCascade Bnd_Box class
struct BndUtils {
    // Extends 'box' with bounding box 'other'
    static void add(Bnd_Box* box, const Bnd_Box& other);

    // Same as Bnd_Box::IsOpen() but provided for OpenCascade < v7.4
    static bool isOpen(const Bnd_Box& bndBox);

    // Same as Bnd_Box::HasFinitePart() but provided for OpenCascade < v7.4
    static bool hasFinitePart(const Bnd_Box& bndBox);

    // Same as Bnd_Box::FinitePart() but provided for OpenCascade < v7.4
    static Bnd_Box finitePart(const Bnd_Box& bndBox);
};

// Raw coordinates of a AA bounding box
struct BndBoxCoords {
    double xmin;
    double ymin;
    double zmin;
    double xmax;
    double ymax;
    double zmax;

    // Returns center point of the bounding box
    gp_Pnt center() const;

    // Returns the 8 vertex points of the bounding box
    std::array<gp_Pnt, 8> vertices() const;

    // Returns (xmin, ymin, zmin) vertex point
    gp_Pnt minVertex() const;

    // Returns (xmax, ymax, zmax) vertex point
    gp_Pnt maxVertex() const;

    // Creates coordinates out of a Bnd_Box objecy
    static BndBoxCoords get(const Bnd_Box& box);
};

} // namespace Mayo
