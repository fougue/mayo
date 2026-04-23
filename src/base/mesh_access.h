/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
    virtual std::optional<Quantity_Color> nodeColor(int i) const = 0;
    virtual const TopLoc_Location& location() const = 0;
    virtual const OccHandle<Poly_Triangulation>& triangulation() const = 0;
};

// Iterates over meshes from `treeNode` and call `fnCallback` for each item.
void IMeshAccess_visitMeshes(
        const DocumentTreeNode& treeNode,
        std::function<void(const IMeshAccess&)> fnCallback
);

} // namespace Mayo
