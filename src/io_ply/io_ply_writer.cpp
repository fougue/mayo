/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_ply_writer.h"

#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/document.h"
#include "../base/label_data.h"
#include "../base/io_system.h"
#include "../base/math_utils.h"
#include "../base/mesh_access.h"
#include "../base/messenger.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <Poly_Triangulation.hxx>

#include <fmt/format.h>
#include <gsl/util>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <locale>
#include <string>

namespace Mayo::IO {

namespace {

enum class Endianness { Unknown, Little, Big };

Endianness hostEndianness()
{
    const uint32_t value = 0x01020408u;
    unsigned char bytes[sizeof(value)];
    std::memcpy(bytes, &value, sizeof(value));
    // Little: bytes = {08, 04, 02, 01}
    // Big   : bytes = {01, 02, 04, 08}

    if (bytes[0] == 0x08 && bytes[3] == 0x01)
        return Endianness::Little;

    if (bytes[0] == 0x01 && bytes[3] == 0x08)
        return Endianness::Big;

    return Endianness::Unknown;
}

} // namespace

PlyWriter::Parameters::Parameters()
{
    this->restoreDefaults();
    this->comment.setDescription(textId("Line that will appear in header"));
}

void PlyWriter::Parameters::restoreDefaults()
{
    this->format.setValue(Format::Binary);
    this->writeColors.setValue(true);
    this->defaultColor.setValue(Quantity_NOC_GRAY);
    this->comment.setValue({});
}

bool PlyWriter::transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();
    m_vecNode.clear();
    m_vecNodeColor.clear();
    m_vecFace.clear();

    // TODO Investigate bad looking 3D mesh when defining vertex colors
    // TODO Investigate task abort issue

    // Count number of faces for progress report
    int count = 0;
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& docTreeNode) {
        if (docTreeNode.isLeaf()) {
            IMeshAccess_visitMeshes(docTreeNode, [&](const IMeshAccess&) { ++count; });
            if (findLabelDataFlags(docTreeNode.label()) & LabelData_HasPointCloudData)
                ++count;
        }
    });

    // Record face meshes
    int iCount = 0;
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& docTreeNode) {
        if (docTreeNode.isLeaf() && !progress->isAbortRequested()) {
            IMeshAccess_visitMeshes(docTreeNode, [&](const IMeshAccess& mesh) {
                this->addMesh(mesh);
                progress->setValue(MathUtils::toPercent(++iCount, 0, count));
            });
        }
    });

    // Record point clouds
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& docTreeNode) {
        if (docTreeNode.isLeaf()
                && (findLabelDataFlags(docTreeNode.label()) & LabelData_HasPointCloudData)
                && !progress->isAbortRequested())
        {
            this->addPointCloud(CafUtils::findAttribute<PointCloudData>(docTreeNode.label()));
            progress->setValue(MathUtils::toPercent(++iCount, 0, count));
        }
    });

    return true;
}

bool PlyWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();
    const bool isBinary = m_params.format == Format::Binary;
    std::ios_base::openmode mode = std::ios_base::out;
    if (isBinary)
        mode |= std::ios_base::binary;

    std::ofstream fstr(filepath, mode);
    if (!fstr.is_open()) {
        this->messenger()->emitError(textIdTr("Failed to open file"));
        return false;
    }

    // Define PLY format
    const char* strPlyFormat = nullptr;
    if (isBinary) {
        const Endianness endian = hostEndianness();
        if (endian == Endianness::Little)
            strPlyFormat = "binary_little_endian";
        else if (endian == Endianness::Big)
            strPlyFormat = "binary_big_endian";
        else
            this->messenger()->emitError(textIdTr("Unknown host endianness"));
    }
    else {
        strPlyFormat = "ascii";
    }

    if (!strPlyFormat)
        return false;

    // Write PLY header
    fstr.imbue(std::locale::classic());
    fstr << "ply\n"
         << "format " << strPlyFormat << " 1.0\n";

    if (!m_params.comment.value().empty()) {
        std::string strComment = m_params.comment;
        std::replace(strComment.begin(), strComment.end(), '\n', ' ');
        std::replace(strComment.begin(), strComment.end(), '\r', ' ');
        fstr << "comment " << m_params.comment.value() << "\n";
    }

    fstr << "element vertex " << m_vecNode.size() << "\n"
         << "property float x\n"
         << "property float y\n"
         << "property float z\n";

    if (m_params.writeColors) {
        fstr << "property uchar red\n"
             << "property uchar green\n"
             << "property uchar blue\n";
    }

    fstr << "element face " << m_vecFace.size() << "\n"
         << "property list uchar int vertex_indices\n"
         << "end_header\n";

    // Helpers for progress report
    const int elementCount = int(m_vecNode.size() + m_vecFace.size());
    int iElement = 0;
    auto fnUpdateProgress = [&]{
        ++iElement;
        if (iElement % 50 == 0) {
            progress->setValue(MathUtils::toPercent(iElement, 0, elementCount));
            if (progress->isAbortRequested())
                return false;
        }

        return true;
    };

    // Write vertices
    for (const Vertex& node : m_vecNode) {
        const auto inode = &node - &m_vecNode.front();
        if (isBinary) {
            fstr.write(reinterpret_cast<const char*>(&node.x), 12);
            if (m_params.writeColors)
                fstr.write(reinterpret_cast<const char*>(&m_vecNodeColor.at(inode).red), 3);
        }
        else {
            fstr << node.x << " " << node.y << " " << node.z;
            if (m_params.writeColors) {
                const Color& c = m_vecNodeColor.at(inode);
                fstr << " " << int(c.red) << " " << int(c.green) << " " << int(c.blue);
            }

            fstr << "\n";
        }

        if (!fnUpdateProgress())
            return true;
    }

    fstr.flush();
    // Write face indices
    for (const Face& face : m_vecFace) {
        if (isBinary) {
            const uint8_t indexCount = 3;
            fstr.write(reinterpret_cast<const char*>(&indexCount), 1);
            fstr.write(reinterpret_cast<const char*>(&face.v1), 12);
        }
        else {
            fstr << "3 " << face.v1 << " " << face.v2 << " " << face.v3 << "\n";
        }

        if (!fnUpdateProgress())
            return true;
    }

    fstr.flush();
    return true;
}

void PlyWriter::addMesh(const IMeshAccess& mesh)
{
    const OccHandle<Poly_Triangulation>& triangulation = mesh.triangulation();
    for (int i = 1; i <= triangulation->NbTriangles(); ++i) {
        const Poly_Triangle& triangle = triangulation->Triangle(i);
        assert(Cpp::cmpLessEqual(m_vecNode.size(), INT32_MAX));
        auto offset = static_cast<int32_t>(m_vecNode.size());
        const Face face{
            offset + triangle(1) - 1, offset + triangle(2) - 1, offset + triangle(3) - 1
        };
        m_vecFace.push_back(std::move(face));
    }

    for (int i = 1; i <= triangulation->NbNodes(); ++i) {
        const Vertex vertex = PlyWriter::toVertex(triangulation->Node(i).Transformed(mesh.location()));
        m_vecNode.push_back(std::move(vertex));
    }

    if (m_params.writeColors) {
        for (int i = 0; i < triangulation->NbNodes(); ++i) {
            const std::optional<Quantity_Color> nodeColor = mesh.nodeColor(i);
            const Quantity_Color& defaultNodeColor = m_params.defaultColor.value();
            m_vecNodeColor.push_back(PlyWriter::toColor(nodeColor.value_or(defaultNodeColor)));
        }
    }
}

void PlyWriter::addPointCloud(const PointCloudDataPtr& pntCloud)
{
    const OccHandle<Graphic3d_ArrayOfPoints>& points = pntCloud->points();
    const int pntCount = points->VertexNumber();
    for (int i = 1; i <= pntCount; ++i) {
        const Vertex vertex = PlyWriter::toVertex(points->Vertice(i));
        m_vecNode.push_back(std::move(vertex));
    }

    if (m_params.writeColors) {
        const bool hasColors = points->HasVertexColors();
        for (int i = 1; i <= pntCount; ++i) {
            const Quantity_Color pntColor = hasColors ? points->VertexColor(i) : m_params.defaultColor.value();
            m_vecNodeColor.push_back(PlyWriter::toColor(pntColor));
        }
    }
}

PlyWriter::Vertex PlyWriter::toVertex(const gp_Pnt& pnt)
{
    return Vertex{
        static_cast<float>(pnt.X()),
        static_cast<float>(pnt.Y()),
        static_cast<float>(pnt.Z())
    };
}

PlyWriter::Color PlyWriter::toColor(const Quantity_Color& c)
{
    const Quantity_Color cc = TKernelUtils::toLinearRgbColor(c);
    return {
        static_cast<uint8_t>(cc.Red() * 255),
        static_cast<uint8_t>(cc.Green() * 255),
        static_cast<uint8_t>(cc.Blue() * 255)
    };
}

} // namespace Mayo::IO
