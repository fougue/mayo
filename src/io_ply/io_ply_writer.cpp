/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <Poly_Triangulation.hxx>

#include <gsl/util>
#include <fmt/format.h>

#include <algorithm>
#include <fstream>
#include <locale>
#include <string>

namespace Mayo::IO {

namespace {

enum class Endianness { Unknown, Little, Big };

Endianness hostEndianness()
{
    union IntBytes32Convert {
        uint32_t integer;
        uint8_t  bytes[4];
    };
    IntBytes32Convert conv;
    conv.integer = 0x01020408;
    if (conv.bytes[0] == 0x08 && conv.bytes[3] == 0x01)
        return Endianness::Little;

    if (conv.bytes[0] == 0x01 && conv.bytes[3] == 0x08)
        return Endianness::Big;

    return Endianness::Unknown;
}

} // namespace

struct PlyWriterI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::PlyWriterI18N)
};

class PlyWriter::Properties : public PropertyGroup {
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->targetFormat.mutableEnumeration().changeTrContext(PlyWriterI18N::textIdContext());
        this->comment.setDescription(PlyWriterI18N::textIdTr("Line that will appear in header"));
    }

    void restoreDefaults() override {
        const PlyWriter::Parameters defaultParams;
        this->targetFormat.setValue(defaultParams.format);
        this->writeColors.setValue(defaultParams.writeColors);
        this->defaultColor.setValue(defaultParams.defaultColor.GetRGB());
        this->comment.setValue(defaultParams.comment);
    }

    PropertyEnum<PlyWriter::Format> targetFormat{ this, PlyWriterI18N::textId("targetFormat") };
    PropertyBool writeColors{ this, PlyWriterI18N::textId("writeColors") };
    PropertyOccColor defaultColor{ this, PlyWriterI18N::textId("defaultColor") };
    PropertyString comment{ this, PlyWriterI18N::textId("comment") };
};

bool PlyWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress)
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
        this->messenger()->emitError(PlyWriterI18N::textIdTr("Failed to open file"));
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
            this->messenger()->emitError(PlyWriterI18N::textIdTr("Unknown host endianness"));
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

    if (!m_params.comment.empty()) {
        std::string strComment = m_params.comment;
        std::replace(strComment.begin(), strComment.end(), '\n', ' ');
        std::replace(strComment.begin(), strComment.end(), '\r', ' ');
        fstr << "comment " << m_params.comment << "\n";
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

std::unique_ptr<PropertyGroup> PlyWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void PlyWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.format = ptr->targetFormat;
        m_params.writeColors = ptr->writeColors;
        m_params.defaultColor = Quantity_ColorRGBA(ptr->defaultColor);
        m_params.comment = ptr->comment;
    }
}

void PlyWriter::addMesh(const IMeshAccess& mesh)
{
    const OccHandle<Poly_Triangulation>& triangulation = mesh.triangulation();
    for (int i = 1; i <= triangulation->NbTriangles(); ++i) {
        const Poly_Triangle& triangle = triangulation->Triangle(i);
        const int32_t offset = CppUtils::safeStaticCast<int32_t>(m_vecNode.size());
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
            const Quantity_Color& defaultNodeColor = m_params.defaultColor.GetRGB();
            m_vecNodeColor.push_back(PlyWriter::toColor(nodeColor ? nodeColor.value() : defaultNodeColor));
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
            const Quantity_Color pntColor = hasColors ? points->VertexColor(i) : m_params.defaultColor.GetRGB();
            m_vecNodeColor.push_back(PlyWriter::toColor(pntColor));
        }
    }
}

PlyWriter::Vertex PlyWriter::toVertex(const gp_Pnt& pnt)
{
    return Vertex{ float(pnt.X()), float(pnt.Y()), float(pnt.Z()) };
}

PlyWriter::Color PlyWriter::toColor(const Quantity_Color& c)
{
    const Quantity_Color cc = TKernelUtils::toLinearRgbColor(c);
    return { uint8_t(cc.Red() * 255), uint8_t(cc.Green() * 255), uint8_t(cc.Blue() * 255) };
}

} // namespace Mayo::IO
