/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_obj_writer.h"

#include "../base/application_item.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/enumeration_fromenum.h"
#include "../base/text_id.h"
#include "io_occ_common.h"

#include <RWObj_CafWriter.hxx>

namespace Mayo {
namespace IO {

class OccObjWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccObjWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->coordinatesConverter.setDescription(
                    textIdTr("Coordinate system transformation from OpenCascade to OBJ"));
    }

    void restoreDefaults() override {
        const Parameters defaults;
        this->coordinatesConverter.setValue(defaults.coordinatesConverter);
    }

    PropertyEnum<RWMesh_CoordinateSystem> coordinatesConverter{ this, textId("coordinatesConverter") };
};

bool OccObjWriter::transfer(Span<const ApplicationItem> spanAppItem, TaskProgress*)
{
    m_document.Nullify();
    m_seqRootLabel.Clear();
    for (const ApplicationItem& appItem : spanAppItem) {
        if (appItem.isDocument() && m_document.IsNull()) {
            m_document = appItem.document();
        }
        else if (appItem.isDocumentTreeNode()) {
            if (m_document.IsNull())
                m_document = appItem.document();

            if (appItem.document().get() == m_document.get())
                m_seqRootLabel.Append(appItem.documentTreeNode().label());
        }
    }

    if (!m_document)
        return false;

    return true;
}

bool OccObjWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (!m_document)
        return false;

    Handle_Message_ProgressIndicator occProgress = new OccProgressIndicator(progress);
    RWObj_CafWriter writer(filepath.u8string().c_str());
    const TColStd_IndexedDataMapOfStringString fileInfo;
    if (m_seqRootLabel.IsEmpty())
        return writer.Perform(m_document, fileInfo, occProgress->Start());
    else
        return writer.Perform(m_document, m_seqRootLabel, nullptr, fileInfo, occProgress->Start());
}

std::unique_ptr<PropertyGroup> OccObjWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccObjWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.coordinatesConverter = ptr->coordinatesConverter;
    }
}

} // namespace IO
} // namespace Mayo
