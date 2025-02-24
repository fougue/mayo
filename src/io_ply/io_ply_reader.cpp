/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_ply_reader.h"

#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/triangulation_annex_data.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/mesh_utils.h"
#include "../base/messenger.h"
#include "../base/point_cloud_data.h"
#include "../base/property_builtins.h"
#include "../base/tkernel_utils.h"
#include "miniply.h"
// TODO Move miniply library files into 3rdparty folder

#include <Poly_Triangulation.hxx>
#include <TDataStd_Name.hxx>

namespace Mayo::IO {

bool PlyReader::readFile(const FilePath& filepath, TaskProgress* /*progress*/)
{
    miniply::PLYReader reader(filepath.u8string().c_str());
    if (!reader.valid())
        return false;

    // Reset internal data
    m_baseFilename = filepath.stem();
    m_nodeCount = 0;
    m_vecNodeCoord.clear();
    m_vecColorComponent.clear();
    m_vecIndex.clear();
    m_vecNormalCoord.clear();
    bool assumeTriangles = true;

    // Guess if PLY faces are triangles
    uint32_t faceIdxs[3] = {};
    if (assumeTriangles) {
        miniply::PLYElement* faceElem = reader.get_element(reader.find_element(miniply::kPLYFaceElement));
        if (faceElem)
            assumeTriangles = faceElem->convert_list_to_fixed_size(faceElem->find_property("vertex_indices"), 3, faceIdxs);
    }

    bool okLoad = true;
    bool gotVerts = false;
    bool gotFaces = false;
    while (reader.has_element() && (!gotVerts || !gotFaces)) {
        if (reader.element_is(miniply::kPLYVertexElement)) {
            uint32_t prop3Idxs[3] = {};
            if (!reader.load_element() || !reader.find_pos(prop3Idxs)) {
                okLoad = false;
                break;
            }

            m_nodeCount = reader.num_rows();
            m_vecNodeCoord.resize(m_nodeCount * 3);
            reader.extract_properties(prop3Idxs, 3, miniply::PLYPropertyType::Float, m_vecNodeCoord.data());
            if (reader.find_normal(prop3Idxs)) {
                m_vecNormalCoord.resize(m_nodeCount * 3);
                reader.extract_properties(prop3Idxs, 3, miniply::PLYPropertyType::Float, m_vecNormalCoord.data());
            }

            if (reader.find_color(prop3Idxs)) {
                m_vecColorComponent.resize(m_nodeCount * 3);
                reader.extract_properties(prop3Idxs, 3, miniply::PLYPropertyType::UChar, m_vecColorComponent.data());
            }

            //if (reader.find_texcoord(propIdxs)) {
            //    vecUvCoord.resize(nodeCount * 2);
            //    reader.extract_properties(propIdxs, 2, miniply::PLYPropertyType::Float, vecUvCoord.data());
            //}

            gotVerts = true;
        }
        else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement)) {
            if (!reader.load_element())
                break;

            if (assumeTriangles) {
                m_vecIndex.resize(reader.num_rows() * 3);
                reader.extract_properties(faceIdxs, 3, miniply::PLYPropertyType::Int, m_vecIndex.data());
            }
            else {
                uint32_t propIdx = 0;
                if (!reader.find_indices(&propIdx))
                    break;

                const bool polys = reader.requires_triangulation(propIdx);
                if (polys && !gotVerts) {
                    this->messenger()->emitError("Face data needing triangulation found before vertex data");
                    break;
                }

                if (polys) {
                    m_vecIndex.resize(reader.num_triangles(propIdx) * 3);
                    reader.extract_triangles(propIdx, m_vecNodeCoord.data(), m_nodeCount, miniply::PLYPropertyType::Int, m_vecIndex.data());
                }
                else {
                    m_vecIndex.resize(reader.num_rows() * 3);
                    reader.extract_list_property(propIdx, miniply::PLYPropertyType::Int, m_vecIndex.data());
                }
            }

            gotFaces = true;
        }
        else if (!gotFaces && reader.element_is("tristrips")) {
            if (!reader.load_element()) {
                this->messenger()->emitError("Failed to load triangle strips");
                break;
            }

            uint32_t propIdx = reader.element()->find_property("vertex_indices");
            if (propIdx == miniply::kInvalidIndex) {
                this->messenger()->emitError("Couldn't find 'vertex_indices' property for the 'tristrips' element");
                break;
            }

            m_vecIndex.resize(reader.sum_of_list_counts(propIdx));
            reader.extract_list_property(propIdx, miniply::PLYPropertyType::Int, m_vecIndex.data());
            gotFaces = true;
        }

        reader.next_element();
    } // endwhile

    return okLoad;
}

TDF_LabelSequence PlyReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    TDF_Label entityLabel;
    if (!m_vecNodeCoord.empty() && !m_vecIndex.empty())
        entityLabel = this->transferMesh(doc, progress);

    if (!m_vecNodeCoord.empty() && m_vecIndex.empty())
        entityLabel = this->transferPointCloud(doc, progress);

    if (!entityLabel.IsNull()) {
        TDataStd_Name::Set(entityLabel, filepathTo<TCollection_ExtendedString>(m_baseFilename));
        return CafUtils::makeLabelSequence({ entityLabel });
    }

    return {};
}

TDF_Label PlyReader::transferMesh(DocumentPtr doc, TaskProgress* /*progress*/)
{
    // Create target mesh
    const int triangleCount = CppUtils::safeStaticCast<int>(m_vecIndex.size() / 3);
    auto mesh = makeOccHandle<Poly_Triangulation>(m_nodeCount, triangleCount, false/*hasUvNodes*/);
    if (!m_vecNormalCoord.empty())
        MeshUtils::allocateNormals(mesh);

    // Copy nodes(vertices) into mesh
    for (int i = 0; CppUtils::cmpLess(i, m_vecNodeCoord.size()); i += 3) {
        const auto& vec = m_vecNodeCoord;
        const gp_Pnt node = { vec.at(i), vec.at(i + 1), vec.at(i + 2) };
        MeshUtils::setNode(mesh, (i / 3) + 1, node);
    }

    // Copy triangles indices into mesh
    for (int i = 0; CppUtils::cmpLess(i, m_vecIndex.size()); i += 3) {
        const auto& vec = m_vecIndex;
        const Poly_Triangle tri = { 1 + vec.at(i), 1 + vec.at(i + 1), 1 + vec.at(i + 2) };
        MeshUtils::setTriangle(mesh, (i / 3) + 1, tri);
    }

    // Copy normals(optional) into mesh
    for (int i = 0; CppUtils::cmpLess(i, m_vecNormalCoord.size()); i += 3) {
        const auto& vec = m_vecNormalCoord;
        const MeshUtils::Poly_Triangulation_NormalType n(vec.at(i), vec.at(i + 1), vec.at(i + 2));
        MeshUtils::setNormal(mesh, (i / 3) + 1, n);
    }

    // Copy colors(optional) into mesh
    std::vector<Quantity_Color> vecColor;
    for (int i = 0; CppUtils::cmpLess(i, m_vecColorComponent.size()); i += 3) {
        const auto& vec = m_vecColorComponent;
        const Quantity_Color color = {
            vec.at(i) / 255., vec.at(i + 1) / 255., vec.at(i + 2) / 255.,
            TKernelUtils::preferredRgbColorType()
        };
        vecColor.push_back(color);
    }

    // Insert mesh as a document entity
    const TDF_Label entityLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(entityLabel, BRepUtils::makeFace(mesh)); // IMPORTANT: pure mesh part marker!
    TriangulationAnnexData::Set(entityLabel, vecColor);
    return entityLabel;
}

TDF_Label PlyReader::transferPointCloud(DocumentPtr doc, TaskProgress* /*progress*/)
{
    const bool hasColors = !m_vecColorComponent.empty();
    const bool hasNormals = false; //!m_vecNormalCoord.empty();
    auto gfxPoints = new Graphic3d_ArrayOfPoints(
        CppUtils::safeStaticCast<int>(m_vecNodeCoord.size()), hasColors, hasNormals
    );

    // Add nodes(vertices) into point cloud
    for (int i = 0; CppUtils::cmpLess(i, m_vecNodeCoord.size()); i += 3) {
        const auto& vec = m_vecNodeCoord;
        const gp_Pnt node = { vec.at(i), vec.at(i + 1), vec.at(i + 2) };
        gfxPoints->AddVertex(node);
    }

    if (hasColors) {
        for (int i = 0; CppUtils::cmpLess(i, m_vecColorComponent.size()); i += 3) {
            const auto& vec = m_vecColorComponent;
            const Quantity_Color color{
                        vec.at(i) / 255., vec.at(i + 1) / 255., vec.at(i + 2) / 255.,
                        TKernelUtils::preferredRgbColorType()
            };
            gfxPoints->SetVertexColor((i / 3) + 1, color);
        }
    }

#if 0
    if (hasNormals) {
        for (int i = 0; CppUtils::cmpLess(i, m_vecNormalCoord.size()); i += 3) {
            const auto& vec = m_vecNormalCoord;
            gfxPoints->SetVertexNormal((i / 3) + 1, vec.at(i), vec.at(i + 1), vec.at(i + 2));
        }
    }
#endif

    // Insert point cloud as a document entity
    const TDF_Label entityLabel = doc->newEntityLabel();
    PointCloudData::Set(entityLabel, gfxPoints);
    return entityLabel;
}

} // namespace Mayo::IO
