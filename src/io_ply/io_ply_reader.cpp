/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_ply_reader.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/data_triangulation.h"
#include "../base/filepath_conv.h"
#include "../base/document.h"
#include "../base/messenger.h"
#include "../base/property_builtins.h"
#include "miniply.h"

#include <Poly_Triangulation.hxx>
#include <TDataStd_Name.hxx>
#include <vector>

namespace Mayo {
namespace IO {

PlyReader::~PlyReader()
{
    delete m_reader;
}

bool PlyReader::readFile(const FilePath& filepath, TaskProgress* /*progress*/)
{
    m_vecElementPtr.clear();
    delete m_reader;
    m_reader = new miniply::PLYReader(filepath.u8string().c_str());
    if (!m_reader->valid())
        return false;

    m_baseFilename = filepath.stem();
    return true;
}

TDF_LabelSequence PlyReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (!m_reader || !m_reader->valid())
        return {};

    bool assumeTriangles = true;
    uint32_t faceIdxs[3] = {};
    if (assumeTriangles) {
        miniply::PLYElement* faceElem = m_reader->get_element(m_reader->find_element(miniply::kPLYFaceElement));
        if (!faceElem)
            return {};

        assumeTriangles = faceElem->convert_list_to_fixed_size(faceElem->find_property("vertex_indices"), 3, faceIdxs);
    }

    uint32_t nodeCount = 0;
    std::vector<float> vecNodeCoord;
    //std::vector<float> vecUvCoord;
    std::vector<uint8_t> vecColorComponent;
    std::vector<int> vecIndex;
    std::vector<float> vecNormalCoord;
    bool gotVerts = false;
    bool gotFaces = false;
    while (m_reader->has_element() && (!gotVerts || !gotFaces)) {
        if (m_reader->element_is(miniply::kPLYVertexElement)) {
            uint32_t propIdxs[3] = {};
            if (!m_reader->load_element() || !m_reader->find_pos(propIdxs))
                break;

            nodeCount = m_reader->num_rows();
            vecNodeCoord.resize(nodeCount * 3);
            m_reader->extract_properties(propIdxs, 3, miniply::PLYPropertyType::Float, vecNodeCoord.data());
            if (m_reader->find_normal(propIdxs)) {
                vecNormalCoord.resize(nodeCount * 3);
                m_reader->extract_properties(propIdxs, 3, miniply::PLYPropertyType::Float, vecNormalCoord.data());
            }

            if (m_reader->find_color(propIdxs)) {
                vecColorComponent.resize(nodeCount * 3);
                m_reader->extract_properties(propIdxs, 3, miniply::PLYPropertyType::UChar, vecColorComponent.data());
            }

//            if (m_reader->find_texcoord(propIdxs)) {
//                vecUvCoord.resize(nodeCount * 2);
//                m_reader->extract_properties(propIdxs, 2, miniply::PLYPropertyType::Float, vecUvCoord.data());
//            }

            gotVerts = true;
        }
        else if (!gotFaces && m_reader->element_is(miniply::kPLYFaceElement)) {
            if (!m_reader->load_element())
                break;

            if (assumeTriangles) {
                vecIndex.resize(m_reader->num_rows() * 3);
                m_reader->extract_properties(faceIdxs, 3, miniply::PLYPropertyType::Int, vecIndex.data());
            }
            else {
                uint32_t propIdx = 0;
                if (!m_reader->find_indices(&propIdx))
                    break;

                const bool polys = m_reader->requires_triangulation(propIdx);
                if (polys && !gotVerts) {
                    this->messenger()->emitError("Face data needing triangulation found before vertex data");
                    break;
                }

                if (polys) {
                    vecIndex.resize(m_reader->num_triangles(propIdx) * 3);
                    m_reader->extract_triangles(propIdx, vecNodeCoord.data(), nodeCount, miniply::PLYPropertyType::Int, vecIndex.data());
                }
                else {
                    vecIndex.resize(m_reader->num_rows() * 3);
                    m_reader->extract_list_property(propIdx, miniply::PLYPropertyType::Int, vecIndex.data());
                }
            }

            gotFaces = true;
        }
        else if (!gotFaces && m_reader->element_is("tristrips")) {
            if (!m_reader->load_element()) {
                this->messenger()->emitError("Failed to load triangle strips");
                break;
            }

            uint32_t propIdx = m_reader->element()->find_property("vertex_indices");
            if (propIdx == miniply::kInvalidIndex) {
                this->messenger()->emitError("Couldn't find 'vertex_indices' property for the 'tristrips' element");
                break;
            }

            vecIndex.resize(m_reader->sum_of_list_counts(propIdx));
            m_reader->extract_list_property(propIdx, miniply::PLYPropertyType::Int, vecIndex.data());
            gotFaces = true;
        }

        m_reader->next_element();
    }

    if (!gotVerts || !gotFaces/* || !trimesh->all_indices_valid()*/)
        return {};

    const int triangleCount = CppUtils::safeStaticCast<int>(vecIndex.size() / 3);
    const bool hasUV = false; /* !vecUvCoord.empty(); */
    const bool hasNormals = !vecNormalCoord.empty();
    Handle_Poly_Triangulation mesh = new Poly_Triangulation(nodeCount, triangleCount, hasUV, hasNormals);
    for (int i = 0; i < vecNodeCoord.size(); i += 3) {
        const gp_Pnt node = { vecNodeCoord.at(i), vecNodeCoord.at(i + 1), vecNodeCoord.at(i + 2) };
        mesh->SetNode((i / 3) + 1, node);
    }

//    for (int i = 0; i < vecUvCoord.size(); i += 2) {
//        const gp_Pnt2d uv = { vecUvCoord.at(i), vecUvCoord.at(i + 1) };
//        mesh->SetUVNode((i / 2) + 1, uv);
//    }

    for (int i = 0; i < vecIndex.size(); i += 3) {
        const Poly_Triangle tri = { 1 + vecIndex.at(i), 1 + vecIndex.at(i + 1), 1 + vecIndex.at(i + 2) };
        mesh->SetTriangle((i / 3) + 1, tri);
    }

    for (int i = 0; i < vecNormalCoord.size(); i += 3) {
        const gp_Vec3f n(vecNormalCoord.at(i), vecNormalCoord.at(i + 1), vecNormalCoord.at(i + 2));
        mesh->SetNormal((i / 3) + 1, n);
    }

    std::vector<Quantity_Color> vecColor;
    for (int i = 0; i < vecColorComponent.size(); i += 3) {
        const Quantity_Color color = {
            vecColorComponent.at(i + 0) / 255.,
            vecColorComponent.at(i + 1) / 255.,
            vecColorComponent.at(i + 2) / 255.,
            Quantity_TOC_sRGB
        };
        vecColor.push_back(color);
    }

    const TDF_Label entityLabel = doc->newEntityLabel();
    DataTriangulation::Set(entityLabel, mesh, vecColor);
    TDataStd_Name::Set(entityLabel, filepathTo<TCollection_ExtendedString>(m_baseFilename));
    return CafUtils::makeLabelSequence({ entityLabel });
}

Span<const Format> PlyFactoryReader::formats() const
{
    static const Format arrayFormat[] = { Format_PLY };
    return arrayFormat;
}

std::unique_ptr<Reader> PlyFactoryReader::create(Format format) const
{
    if (format == Format_PLY)
        return std::make_unique<PlyReader>();

    return {};
}

std::unique_ptr<PropertyGroup> PlyFactoryReader::createProperties(Format format, PropertyGroup* parentGroup) const
{
    return {};
}

} // namespace IO
} // namespace Mayo
