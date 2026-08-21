/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_obj_writer.h"

#include "../base/application_item.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/occ_progress_indicator.h"
#include "../base/occt_ncollection_indexed_datamap_of_stringstring.h"
#include "io_occ_common.h"

#include <RWObj_CafWriter.hxx>

namespace Mayo::IO {

OccObjWriter::Parameters::Parameters()
{
    this->restoreDefaults();
    this->inputCoordinateSystem.setDescription(textId("Source coordinate system transformation"));
    this->outputCoordinateSystem.setDescription(textId("Target coordinate system transformation"));
}

void OccObjWriter::Parameters::restoreDefaults()
{
    this->inputCoordinateSystem.setValue(RWMesh_CoordinateSystem_Undefined);
    this->outputCoordinateSystem.setValue(RWMesh_CoordinateSystem_glTF);
}

bool OccObjWriter::transfer(gsl::span<const ApplicationItem> spanAppItem, TaskProgress*)
{
    m_document.Nullify();
    m_seqRootLabel.Clear();
    System::visitUniqueItems(spanAppItem, [=](const ApplicationItem& appItem) {
        if (appItem.isDocument() && m_document.IsNull()) {
            m_document = appItem.document();
        }
        else if (appItem.isDocumentTreeNode()) {
            if (m_document.IsNull())
                m_document = appItem.document();

            if (appItem.document().get() == m_document.get())
                m_seqRootLabel.Append(appItem.documentTreeNode().label());
        }
    });

    if (!m_document)
        return false;

    return true;
}

bool OccObjWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (!m_document)
        return false;

    auto occProgress = makeOccHandle<OccProgressIndicator>(progress);
    RWObj_CafWriter writer(filepath.u8string().c_str());
    writer.ChangeCoordinateSystemConverter().SetInputCoordinateSystem(m_params.inputCoordinateSystem);
    writer.ChangeCoordinateSystemConverter().SetOutputCoordinateSystem(m_params.outputCoordinateSystem);
    const NCollection_IndexedDataMapOfStringString fileInfo;
    try {
        if (m_seqRootLabel.IsEmpty())
            return writer.Perform(m_document, fileInfo, occProgress->Start());
        else
            return writer.Perform(m_document, m_seqRootLabel, nullptr, fileInfo, occProgress->Start());
    }
    catch (const Standard_Failure& err) {
        this->messenger()->error() << TKernelUtils::errorMessage(err);
    }

    return false;
}

} // namespace Mayo::IO
