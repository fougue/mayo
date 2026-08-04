/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_brep.h"

#include "../base/application_item.h"
#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/occ_progress_indicator.h"
#include "../base/io_system.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <BinTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <Standard_Version.hxx>
#include <TDataStd_Name.hxx>

namespace Mayo::IO {

OccBRepReader::OccBRepReader(bool binaryMode) : m_isBinary(binaryMode) {
    //
}

bool OccBRepReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    m_shape.Nullify();
    m_baseFilename = filepath.stem();

    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    if (m_isBinary) {
        return BinTools::Read(
            m_shape,
            filepath.u8string().c_str()
#if OCC_VERSION_HEX > 0x070500
            , TKernelUtils::start(indicator)
#endif
        );
    }

    BRep_Builder brepBuilder;
    return BRepTools::Read(
        m_shape,
        filepath.u8string().c_str(),
        brepBuilder,
        TKernelUtils::start(indicator)
    );
}

NCollection_Sequence<TDF_Label> OccBRepReader::transfer(DocumentPtr doc, TaskProgress* /*progress*/)
{
    if (m_shape.IsNull())
        return {};

    const OccHandle<XCAFDoc_ShapeTool> shapeTool = doc->xcaf().shapeTool();
    const TDF_Label labelShape = shapeTool->NewShape();
    shapeTool->SetShape(labelShape, m_shape);
    TDataStd_Name::Set(labelShape, filepathTo<TCollection_ExtendedString>(m_baseFilename));
    return CafUtils::makeLabelSequence({ labelShape });
}

OccBRepWriter::OccBRepWriter(bool binaryMode) : m_isBinary(binaryMode) {
    //
}

bool OccBRepWriter::transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
    m_shape = TopoDS_Shape();

    std::vector<TopoDS_Shape> vecShape;
    vecShape.reserve(appItems.size());
    System::visitUniqueItems(appItems, [&](const ApplicationItem& item) {
        if (item.isDocument()) {
            for (const TDF_Label& label : item.document()->xcaf().topLevelFreeShapes())
                vecShape.push_back(XCaf::shape(label));
        }
        else if (item.isDocumentTreeNode()) {
            const TDF_Label labelNode = item.documentTreeNode().label();
            vecShape.push_back(XCaf::shape(labelNode));
        }
    });

    if (vecShape.size() > 1) {
        m_shape = BRepUtils::makeEmptyCompound();
        for (const TopoDS_Shape& subShape : vecShape)
            BRepUtils::addShape(&m_shape, subShape);
    }
    else if (vecShape.size() == 1) {
        m_shape = vecShape.front();
    }

    return true;
}

bool OccBRepWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    if (m_isBinary) {
        return BinTools::Write(
            m_shape,
            filepath.u8string().c_str()
#if OCC_VERSION_HEX > 0x070500
            , TKernelUtils::start(indicator)
#endif
        );
    }

    return BRepTools::Write(m_shape, filepath.u8string().c_str(), TKernelUtils::start(indicator));
}

} // namespace Mayo::IO
