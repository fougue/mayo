/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// --
// -- Basically the same as XSDRAWSTLVRML_DataSource but it allows to be free of TKXSDRAW
// --

#include "../base/occ_handle.h"

#include <MeshVS_DataSource.hxx>
#include <MeshVS_EntityType.hxx>
#include <Poly_Triangulation.hxx>
#include <TColStd_PackedMapOfInteger.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>

namespace Mayo {

class GraphicsMeshDataSource : public MeshVS_DataSource {
public:
    GraphicsMeshDataSource(const OccHandle<Poly_Triangulation>& mesh);

    bool GetGeom(const int ID, const bool IsElement, TColStd_Array1OfReal& Coords, int& NbNodes, MeshVS_EntityType& Type) const override;
    bool GetGeomType(const int ID, const bool IsElement, MeshVS_EntityType& Type) const override;
    Standard_Address GetAddr(const int /*ID*/, const bool /*IsElement*/) const override { return nullptr; }
    bool GetNodesByElement(const int ID, TColStd_Array1OfInteger& NodeIDs, int& NbNodes) const override;
    const TColStd_PackedMapOfInteger& GetAllNodes() const override { return m_nodes; }
    const TColStd_PackedMapOfInteger& GetAllElements() const override { return m_elements; }
    bool GetNormal(const int Id, const int Max, double& nx, double& ny, double& nz) const override;

private:
  OccHandle<Poly_Triangulation> m_mesh;
  TColStd_PackedMapOfInteger m_nodes;
  TColStd_PackedMapOfInteger m_elements;
  OccHandle<TColStd_HArray2OfInteger> m_elemNodes;
  OccHandle<TColStd_HArray2OfReal> m_nodeCoords;
  OccHandle<TColStd_HArray2OfReal> m_elemNormals;
};

} // namespace Mayo
