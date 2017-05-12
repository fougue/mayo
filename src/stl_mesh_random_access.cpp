#include "stl_mesh_random_access.h"

namespace occ {

StlMeshRandomAccess::StlMeshRandomAccess(const Handle_StlMesh_Mesh &hnd)
    : m_mesh(hnd),
      m_domainCount(!m_mesh.IsNull() ? m_mesh->NbDomains() : 0)
{
    // Count triangles
    m_triangleCount = 0;
    for (int i = 1; i <= m_domainCount; ++i)
        m_triangleCount += m_mesh->NbTriangles(i);
    // Fill vector of triangle data
    m_vecDomainData.resize(m_domainCount);
    m_vecTriangleData.resize(m_triangleCount);
    std::size_t vecTriId = 0;
    for (int domId = 1; domId <= m_domainCount; ++domId) {
        // Cache vertex indexes
        //   TColgp_SequenceOfXYZ::Value(int) is slow(linear search)
        const TColgp_SequenceOfXYZ& seqVertex = m_mesh->Vertices(domId);
        struct DomainData& domData = m_vecDomainData.at(domId - 1);
        domData.vecCoords.reserve(seqVertex.Length());
        for (const gp_XYZ& coords : seqVertex)
            domData.vecCoords.emplace_back(&coords);
        m_vertexCount += seqVertex.Length();
        // Cache triangles
        const StlMesh_SequenceOfMeshTriangle& seqTriangle =
                m_mesh->Triangles(domId);
        for (const Handle_StlMesh_MeshTriangle hndTri : seqTriangle) {
            struct TriangleData& triData = m_vecTriangleData.at(vecTriId);
            triData.ptrTriangle = hndTri.operator->();
            triData.ptrDomain = &domData;
            ++vecTriId;
        }
    }
}

StlMeshRandomAccess::TriangleCoords
StlMeshRandomAccess::triangleCoords(std::size_t index) const
{
    const TriangleData& triData = m_vecTriangleData.at(index);
    const std::vector<const gp_XYZ*>& vecCoords = triData.ptrDomain->vecCoords;
    int iv1, iv2, iv3;
    double nx, ny, nz;
    triData.ptrTriangle->GetVertexAndOrientation(iv1, iv2, iv3, nx, ny, nz);
    return { vecCoords.at(iv1 - 1),
             vecCoords.at(iv2 - 1),
             vecCoords.at(iv3 - 1),
             gp_XYZ(nx, ny, nz) };
}

} // namespace occ
