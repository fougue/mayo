/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TDataStd_Name.hxx>

namespace Mayo::IO {

bool OccBRepReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    m_shape.Nullify();
    m_baseFilename = filepath.stem();
    BRep_Builder brepBuilder;
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    return BRepTools::Read(
        m_shape,
        filepath.u8string().c_str(),
        brepBuilder,
        TKernelUtils::start(indicator)
    );
}

TDF_LabelSequence OccBRepReader::transfer(DocumentPtr doc, TaskProgress* /*progress*/)
{
    if (m_shape.IsNull())
        return {};

    const OccHandle<XCAFDoc_ShapeTool> shapeTool = doc->xcaf().shapeTool();
    const TDF_Label labelShape = shapeTool->NewShape();
    shapeTool->SetShape(labelShape, m_shape);
    TDataStd_Name::Set(labelShape, filepathTo<TCollection_ExtendedString>(m_baseFilename));
    return CafUtils::makeLabelSequence({ labelShape });
}

bool OccBRepWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
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
    return BRepTools::Write(m_shape, filepath.u8string().c_str(), TKernelUtils::start(indicator));
}

} // namespace Mayo::IO
