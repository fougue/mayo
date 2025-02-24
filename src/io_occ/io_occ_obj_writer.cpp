/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_obj_writer.h"

#include "../base/application_item.h"
#include "../base/enumeration_fromenum.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/text_id.h"
#include "io_occ_common.h"

#include <RWObj_CafWriter.hxx>

namespace Mayo::IO {

struct OccObjWriterI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccObjWriterI18N) };

class OccObjWriter::Properties : public PropertyGroup {
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->inputCoordinateSystem.setDescription(
            OccObjWriterI18N::textIdTr("Source coordinate system transformation")
        );
        this->outputCoordinateSystem.setDescription(
            OccObjWriterI18N::textIdTr("Target coordinate system transformation")
        );
    }

    void restoreDefaults() override {
        const Parameters defaults;
        this->inputCoordinateSystem.setValue(defaults.inputCoordinateSystem);
        this->outputCoordinateSystem.setValue(defaults.outputCoordinateSystem);
    }

    PropertyEnum<RWMesh_CoordinateSystem> inputCoordinateSystem{ this, OccObjWriterI18N::textId("inputCoordinateSystem") };
    PropertyEnum<RWMesh_CoordinateSystem> outputCoordinateSystem{ this, OccObjWriterI18N::textId("outputCoordinateSystem") };
};

bool OccObjWriter::transfer(Span<const ApplicationItem> spanAppItem, TaskProgress*)
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
    const TColStd_IndexedDataMapOfStringString fileInfo;
    try {
        if (m_seqRootLabel.IsEmpty())
            return writer.Perform(m_document, fileInfo, occProgress->Start());
        else
            return writer.Perform(m_document, m_seqRootLabel, nullptr, fileInfo, occProgress->Start());
    }
    catch (const Standard_Failure& err) {
        this->messenger()->error() << err.GetMessageString();
    }

    return false;
}

std::unique_ptr<PropertyGroup> OccObjWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccObjWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.inputCoordinateSystem = ptr->inputCoordinateSystem;
        m_params.outputCoordinateSystem = ptr->outputCoordinateSystem;
    }
}

} // namespace Mayo::IO
