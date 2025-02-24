/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_off_writer.h"

#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/label_data.h"
#include "../base/io_system.h"
#include "../base/mesh_access.h"
#include "../base/messenger.h"
#include "../base/property_builtins.h"
#include "../base/task_progress.h"
#include "../base/text_id.h"

#include <Poly_Triangulation.hxx>

#include <fstream>
#include <locale>
#include <string>

namespace Mayo::IO {

struct OffWriterI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OffWriterI18N) };

bool OffWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
    m_vecTreeNode.clear();
    m_vecTreeNode.reserve(appItems.size());
    System::traverseUniqueItems(appItems, [&](const DocumentTreeNode& treeNode) {
        if (treeNode.isLeaf())
            m_vecTreeNode.push_back(treeNode);
    });
    return true;
}

bool OffWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    progress = progress ? progress : &TaskProgress::null();
    std::ofstream fstr(filepath);
    if (!fstr.is_open()) {
        this->messenger()->emitError(OffWriterI18N::textIdTr("Failed to open file"));
        return false;
    }

    fstr.imbue(std::locale::classic());
    fstr << "OFF\n";

    // Count vertices and facets
    int vertexCount = 0;
    int facetCount = 0;
    for (const DocumentTreeNode& treeNode : m_vecTreeNode) {
        IMeshAccess_visitMeshes(treeNode, [&](const IMeshAccess& mesh) {
            vertexCount += mesh.triangulation()->NbNodes();
            facetCount += mesh.triangulation()->NbTriangles();
        });
    }

    // Helper function for progress report
    auto fnUpdateProgress = [=](int current) {
        const auto total = vertexCount + facetCount;
        if (current % 100 || current >= total)
            progress->setValue(MathUtils::toPercent(current, 0, total));
    };

    fstr << vertexCount << " " << facetCount << " " << 0/*edgeCount*/ << "\n";
    // Write vertices
    int ivertex = 0;
    for (const DocumentTreeNode& treeNode : m_vecTreeNode) {
        IMeshAccess_visitMeshes(treeNode, [&](const IMeshAccess& mesh) {
            const gp_Trsf& meshTrsf = mesh.location().Transformation();
            const OccHandle<Poly_Triangulation>& triangulation = mesh.triangulation();
            for (int i = 1; i <= triangulation->NbNodes(); ++i) {
                const gp_Pnt pnt = triangulation->Node(i).Transformed(meshTrsf);
                const std::optional<Quantity_Color> color = mesh.nodeColor(i - 1);
                fstr << pnt.X() << " " << pnt.Y() << " " << pnt.Z();
                if (color.has_value()) {
                    //fstr << " " << int(color->Red()   * 255)
                    //     << " " << int(color->Green() * 255)
                    //     << " " << int(color->Blue()  * 255);
                    fstr << " " << color->Red() << " " << color->Green() << " " << color->Blue();
                }

                fstr << "\n";
                fnUpdateProgress(++ivertex);
            }
        });
    }

    // Write facets(triangles)
    int offsetVertex = 0;
    int ifacet = 0;
    for (const DocumentTreeNode& treeNode : m_vecTreeNode) {
        IMeshAccess_visitMeshes(treeNode, [&](const IMeshAccess& mesh) {
            const OccHandle<Poly_Triangulation>& triangulation = mesh.triangulation();
            for (int i = 1; i <= triangulation->NbTriangles(); ++i) {
                const Poly_Triangle& tri = triangulation->Triangle(i);
                fstr << "3 "
                     << offsetVertex + tri.Value(1) - 1 << " "
                     << offsetVertex + tri.Value(2) - 1 << " "
                     << offsetVertex + tri.Value(3) - 1 << "\n";
                fnUpdateProgress(vertexCount + (++ifacet));
            }

            offsetVertex += triangulation->NbNodes();
        });
    }

    return true;
}

void OffWriter::applyProperties(const PropertyGroup*)
{
}

} // namespace Mayo::IO
