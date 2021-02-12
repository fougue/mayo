/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_mesh_data_source.h"

#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TColgp_SequenceOfXYZ.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>
#include <TColStd_DataMapOfIntegerReal.hxx>

namespace Mayo {

GraphicsMeshDataSource::GraphicsMeshDataSource(const Handle_Poly_Triangulation& mesh)
    : m_mesh(mesh)
{
    if (!m_mesh.IsNull()) {
        const TColgp_Array1OfPnt& aCoords = m_mesh->Nodes();
        const int lenCoords = aCoords.Length();
        m_nodeCoords = new TColStd_HArray2OfReal(1, lenCoords, 1, 3);

        for(int i = 1; i <= lenCoords; ++i) {
            m_nodes.Add(i);
            const gp_XYZ& xyz = aCoords(i).XYZ();
            m_nodeCoords->SetValue(i, 1, xyz.X());
            m_nodeCoords->SetValue(i, 2, xyz.Y());
            m_nodeCoords->SetValue(i, 3, xyz.Z());
        }

        const Poly_Array1OfTriangle& aSeq = m_mesh->Triangles();
        const int lenTriangles = aSeq.Length();
        m_elemNormals = new TColStd_HArray2OfReal(1, lenTriangles, 1, 3);
        m_elemNodes = new TColStd_HArray2OfInteger(1, lenTriangles, 1, 3);

        for(int i = 1; i <= lenTriangles; ++i ) {
            m_elements.Add(i);
            const Poly_Triangle& aTri = aSeq(i);

            int V[3];
            aTri.Get(V[0], V[1], V[2]);

            const gp_Pnt& aP1 = aCoords(V[0]);
            const gp_Pnt& aP2 = aCoords(V[1]);
            const gp_Pnt& aP3 = aCoords(V[2]);

            const gp_Vec aV1(aP1, aP2);
            const gp_Vec aV2(aP2, aP3);

            gp_Vec aN = aV1.Crossed(aV2);
            if (aN.SquareMagnitude() > Precision::SquareConfusion())
                aN.Normalize();
            else
                aN.SetCoord(0.0, 0.0, 0.0);

            for (int j = 0; j < 3; ++j)
                m_elemNodes->SetValue(i, j+1, V[j]);

            m_elemNormals->SetValue (i, 1, aN.X());
            m_elemNormals->SetValue (i, 2, aN.Y());
            m_elemNormals->SetValue (i, 3, aN.Z());
        }
    }
}

bool GraphicsMeshDataSource::GetGeom(
        const int ID,
        const bool IsElement,
        TColStd_Array1OfReal& Coords,
        int& NbNodes,
        MeshVS_EntityType& Type) const
{
    if (m_mesh.IsNull())
        return false;

    if (IsElement) {
        if (ID >= 1 && ID <= m_elements.Extent()) {
            Type = MeshVS_ET_Face;
            NbNodes = 3;
            for (int i = 1, k = 1; i <= 3; ++i) {
                const int IdxNode = m_elemNodes->Value(ID, i);
                for (int j = 1; j <= 3; ++j, ++k)
                    Coords(k) = m_nodeCoords->Value(IdxNode, j);
            }

            return true;
        }

        return false;
    }
    else {
        if (ID >= 1 && ID <= m_nodes.Extent()) {
            Type = MeshVS_ET_Node;
            NbNodes = 1;

            Coords(1) = m_nodeCoords->Value(ID, 1);
            Coords(2) = m_nodeCoords->Value(ID, 2);
            Coords(3) = m_nodeCoords->Value(ID, 3);
            return true;
        }

        return false;
    }
}

bool GraphicsMeshDataSource::GetGeomType(const int, const bool IsElement, MeshVS_EntityType& Type) const
{
    Type = IsElement ? MeshVS_ET_Face : MeshVS_ET_Node;
    return true;
}

bool GraphicsMeshDataSource::GetNodesByElement(const int ID, TColStd_Array1OfInteger& theNodeIDs, int& /*theNbNodes*/) const
{
    if (m_mesh.IsNull())
        return false;

    if (ID >= 1 && ID <= m_elements.Extent() && theNodeIDs.Length() >= 3) {
        const int aLow = theNodeIDs.Lower();
        theNodeIDs(aLow)     = m_elemNodes->Value(ID, 1);
        theNodeIDs(aLow + 1) = m_elemNodes->Value(ID, 2);
        theNodeIDs(aLow + 2) = m_elemNodes->Value(ID, 3);
        return true;
    }

    return false;
}

bool GraphicsMeshDataSource::GetNormal(const int Id, const int Max, double& nx, double& ny, double& nz) const
{
    if (m_mesh.IsNull())
        return false;

    if (Id >= 1 && Id <= m_elements.Extent() && Max >= 3) {
        nx = m_elemNormals->Value(Id, 1);
        ny = m_elemNormals->Value(Id, 2);
        nz = m_elemNormals->Value(Id, 3);
        return true;
    }

    return false;
}

} // namespace Mayo
