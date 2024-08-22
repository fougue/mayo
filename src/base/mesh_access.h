/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// Base
#include "occ_handle.h"
class DocumentTreeNode;

// OpenCascade
#include <Quantity_Color.hxx>
#include <Standard_Handle.hxx>
class Poly_Triangulation;
class TopLoc_Location;

// CppStd
#include <functional>
#include <optional>

namespace Mayo {

// Provides an interface to access mesh geometry
class IMeshAccess {
public:
    // Returns the color at mesh node(or "vertex") of index `i`
    virtual std::optional<Quantity_Color> nodeColor(int i) const = 0;

    // Returns the complete(or "absolute") location of the mesh in the assembly tree
    // This includes as the basis location(actually being last component)
    virtual const TopLoc_Location& absoluteLocation() const = 0;

    // Returns the fundamental(or "basis") location of the mesh, independant of the parent nodes in
    // the assembly tree
    virtual const TopLoc_Location& basisLocation() const = 0;

    // Returns the Poly_Triangulation supporting object
    virtual const OccHandle<Poly_Triangulation>& triangulation() const = 0;
};

// Iterates over meshes from `treeNode` and call `fnCallback` for each item
void IMeshAccess_visitMeshes(
        const DocumentTreeNode& treeNode,
        std::function<void(const IMeshAccess&)> fnCallback
);

} // namespace Mayo
