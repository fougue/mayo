/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_item.h"
#include <Poly_Triangulation.hxx>

namespace Mayo {

class MeshItem : public PartItem {
public:
    MeshItem();

    const Handle_Poly_Triangulation& triangulation() const;
    void setTriangulation(const Handle_Poly_Triangulation& mesh);

    bool isNull() const override;

    static const char TypeName[];
    const char* dynTypeName() const override;

    PropertyInt propertyNodeCount; // Read-only
    PropertyInt propertyTriangleCount; // Read-only

private:
    Handle_Poly_Triangulation m_triangulation;
};

} // namespace Mayo
