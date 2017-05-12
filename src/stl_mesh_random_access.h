#pragma once

#include <Standard_Version.hxx>
#include <StlMesh_Mesh.hxx>
#include <vector>

#if OCC_VERSION_HEX < 0x070000
#  error StlMeshRandomAccess requires OpenCascade >= v7.0.0
#endif

namespace occ {

class StlMeshRandomAccess
{
public:
    explicit StlMeshRandomAccess(const Handle_StlMesh_Mesh& hnd);

    int domainCount() const { return m_domainCount; }
    int triangleCount() const { return m_triangleCount; }
    int vertexCount() const { return m_vertexCount; }

    struct TriangleCoords {
        const gp_XYZ* p1;
        const gp_XYZ* p2;
        const gp_XYZ* p3;
        const gp_XYZ n;
    };
    TriangleCoords triangleCoords(std::size_t index) const;

private:
    struct DomainData {
        std::vector<const gp_XYZ*> vecCoords;
    };

    struct TriangleData {
        const StlMesh_MeshTriangle* ptrTriangle;
        const DomainData* ptrDomain;
    };

    Handle_StlMesh_Mesh m_mesh;
    int m_domainCount = 0;
    int m_triangleCount = 0;
    int m_vertexCount = 0;
    std::vector<DomainData> m_vecDomainData;
    std::vector<TriangleData> m_vecTriangleData;
};

} // namespace occ
