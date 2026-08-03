/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_xcaf.h"
#include "io_occ_caf.h"
#include "../base/occ_handle.h"
#include "../base/string_conv.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"
#include "../base/occ_progress_indicator.h"

#include <BinXCAFDrivers.hxx>
#include <TDocStd_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Editor.hxx>
#include <XmlXCAFDrivers.hxx>

#include <fmt/format.h>
#include <stdexcept>

namespace Mayo::IO {

OccXCafReader::OccXCafReader()
{
    //
}

bool OccXCafReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);

    try {
        m_app = makeOccHandle<TDocStd_Application>();
        XmlXCAFDrivers::DefineFormat(m_app); // to load XML files
        BinXCAFDrivers::DefineFormat(m_app); // to load XBF files
    } catch (const Standard_Failure& ) {
        return false;
    }

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    const PCDM_ReaderStatus error = m_app->Open(filepath.u8string().c_str(), m_doc, indicator->Start());
#else
    const PCDM_ReaderStatus error = m_app->Open(filepath.u8string().c_str(), m_doc);
#endif
    return error == PCDM_RS_OK;
}

NCollection_Sequence<TDF_Label> OccXCafReader::transfer(DocumentPtr doc, TaskProgress* )
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    MayoIO_CafGlobalScopedLock(cafLock);

    const NCollection_Sequence<TDF_Label> seqMark = doc->xcaf().topLevelFreeShapes();

    NCollection_Sequence<TDF_Label> newRoots;
    XCAFDoc_DocumentTool::ShapeTool(m_doc->Main())->GetFreeShapes(newRoots);

    // ideally, it would be better just swapping the documents when target doc is empty,
    // and therefore avoid copying labels and possible bugs in XCAFDoc_Editor,
    // but this would require tricky changes in Mayo::IO API
    XCAFDoc_Editor::Extract(newRoots, doc->Main());

    m_doc->Main().Root().ForgetAllAttributes(true);
    m_app->Close(m_doc);
    m_doc.Nullify();
    m_app.Nullify();

    return doc->xcaf().diffTopLevelFreeShapes(seqMark);
#else
    return false;
#endif
}

} // namespace Mayo::IO
