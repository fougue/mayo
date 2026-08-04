/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_xcaf.h"

#include "../base/occ_progress_indicator.h"
#include "../base/messenger.h"
#include "../base/meta_enum.h"
#include "../base/task_progress.h"
#include "../base/text_id.h"
#include "../base/tkernel_utils.h"
#include "io_occ_caf.h"

#include <BinXCAFDrivers.hxx>
#include <TDocStd_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Editor.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XmlXCAFDrivers.hxx>

#include <fmt/format.h>
#include <stdexcept>

namespace Mayo::IO {

struct OccXCafI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccXCafI18N)
};

OccXCafReader::~OccXCafReader()
{
    this->clearInternals();
}

bool OccXCafReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    MayoIO_CafGlobalScopedLock(cafLock);

    this->clearInternals();
    try {
        m_app = makeOccHandle<TDocStd_Application>();
        XmlXCAFDrivers::DefineFormat(m_app); // to load XML files
        BinXCAFDrivers::DefineFormat(m_app); // to load XBF files
    }
    catch (const Standard_Failure& err) {
        this->messenger()->emitError(fmt::format(
            OccXCafI18N::textIdTr("{} [{}]"), TKernelUtils::errorMessage(err), TKernelUtils::errorTypeName(err)
        ));
        return false;
    }

    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    auto error = m_app->Open(filepath.u8string().c_str(), m_doc, TKernelUtils::start(indicator));
    if (error != PCDM_RS_OK) {
        this->messenger()->emitError(MetaEnum::nameWithoutPrefix(error, "PCDM_RS_"));
        return false;
    }

    return true;
}

NCollection_Sequence<TDF_Label> OccXCafReader::transfer(DocumentPtr doc, TaskProgress* )
{
    MayoIO_CafGlobalScopedLock(cafLock);

    const NCollection_Sequence<TDF_Label> seqMark = doc->xcaf().topLevelFreeShapes();

    NCollection_Sequence<TDF_Label> newRoots;
    XCAFDoc_DocumentTool::ShapeTool(m_doc->Main())->GetFreeShapes(newRoots);

    // Ideally, it would be better just swapping the documents when target doc is empty, and
    // therefore avoid copying labels and possible bugs in XCAFDoc_Editor
    // But this would require tricky changes in Mayo::IO API
    XCAFDoc_Editor::Extract(newRoots, doc->Main());

    return doc->xcaf().diffTopLevelFreeShapes(seqMark);
}

void OccXCafReader::clearInternals()
{
    if (m_doc)
        m_doc->Main().Root().ForgetAllAttributes(true);

    if (m_app)
        m_app->Close(m_doc);

    m_doc.Nullify();
    m_app.Nullify();
}

} // namespace Mayo::IO
