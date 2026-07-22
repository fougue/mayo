/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

// --
// -- Basically the same as XSDRAWSTLVRML_DataSource but it allows to be free of TKXSDRAW
// --

#include "../base/occ_handle.h"
#include "../base/occt_ncollection_harray2_of_builtintypes.h"

#include <MeshVS_DataSource.hxx>
#include <MeshVS_EntityType.hxx>
#include <Poly_Triangulation.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

namespace Mayo {

class GraphicsMeshDataSource : public MeshVS_DataSource {
public:
    explicit GraphicsMeshDataSource(const OccHandle<Poly_Triangulation>& mesh);

    bool GetGeom(const int ID, const bool IsElement, NCollection_Array1<double>& Coords, int& NbNodes, MeshVS_EntityType& Type) const override;
    bool GetGeomType(const int ID, const bool IsElement, MeshVS_EntityType& Type) const override;
    void* GetAddr(const int /*ID*/, const bool /*IsElement*/) const override { return nullptr; }
    bool GetNodesByElement(const int ID, NCollection_Array1<int>& NodeIDs, int& NbNodes) const override;
    const TColStd_PackedMapOfInteger& GetAllNodes() const override { return m_nodes; }
    const TColStd_PackedMapOfInteger& GetAllElements() const override { return m_elements; }
    bool GetNormal(const int Id, const int Max, double& nx, double& ny, double& nz) const override;

private:
  OccHandle<Poly_Triangulation> m_mesh;
  TColStd_PackedMapOfInteger m_nodes;
  TColStd_PackedMapOfInteger m_elements;
  OccHandle<NCollection_HArray2OfInteger> m_elemNodes;
  OccHandle<NCollection_HArray2OfReal> m_nodeCoords;
  OccHandle<NCollection_HArray2OfReal> m_elemNormals;
};

} // namespace Mayo
