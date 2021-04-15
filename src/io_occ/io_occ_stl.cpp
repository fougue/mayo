/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_stl.h"

#include "../base/application_item.h"
#include "../base/document.h"
#include "../base/caf_utils.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_enumeration.h"
#include "../base/string_utils.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <QtCore/QFileInfo>
#include <BRep_Builder.hxx>
#include <RWStl.hxx>
#include <StlAPI_Writer.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <TopoDS_Compound.hxx>

namespace Mayo {
namespace IO {

namespace {

static TopoDS_Shape asShape(const DocumentPtr& doc)
{
    TopoDS_Shape shape;
    const TDF_LabelSequence seqFreeShape = doc->xcaf().topLevelFreeShapes();
    if (seqFreeShape.Size() > 1) {
        TopoDS_Compound cmpd;
        BRep_Builder builder;
        builder.MakeCompound(cmpd);
        for (const TDF_Label& label : seqFreeShape)
            builder.Add(cmpd, XCaf::shape(label));

        shape = cmpd;
    }
    else if (seqFreeShape.Size() == 1) {
        shape = XCaf::shape(seqFreeShape.First());
    }

    return shape;
}

} // namespace

class OccStlWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccStlWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->targetFormat.mutableEnumeration().changeTrContext(this->textIdContext());
    }

    void restoreDefaults() override {
        this->targetFormat.setValue(Format::Binary);
    }

    PropertyEnum<OccStlWriter::Format> targetFormat{ this, textId("targetFormat") };
};

bool OccStlReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    m_baseFilename = filepath.stem();
    m_mesh = RWStl::ReadFile(filepath.u8string().c_str(), TKernelUtils::start(indicator));
    return !m_mesh.IsNull();
}

TDF_LabelSequence OccStlReader::transfer(DocumentPtr doc, TaskProgress* /*progress*/)
{
    if (m_mesh.IsNull())
        return {};

    const TDF_Label entityLabel = doc->newEntityLabel();
    TDataXtd_Triangulation::Set(entityLabel, m_mesh);
    CafUtils::setLabelAttrStdName(entityLabel, filepathTo<QString>(m_baseFilename));
    return CafUtils::makeLabelSequence({ entityLabel });
}

bool OccStlWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
//    if (appItems.size() > 1)
//        return Result::error(tr("OpenCascade RWStl does not support multi-solids"));

    m_shape = {};
    m_mesh = {};
    if (!appItems.empty()) {
        const ApplicationItem& item = appItems.front();
        if (item.isDocument()) {
            m_shape = asShape(item.document());
        }
        else if (item.isDocumentTreeNode()) {
            const TDF_Label label = item.documentTreeNode().label();
            if (XCaf::isShape(label)) {
                m_shape = XCaf::shape(label);
            }
            else {
                auto attrPolyTri = CafUtils::findAttribute<TDataXtd_Triangulation>(label);
                if (!attrPolyTri.IsNull())
                    m_mesh = attrPolyTri->Get();
            }
        }
    }

    return !m_shape.IsNull() || !m_mesh.IsNull();
}

bool OccStlWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (!m_shape.IsNull()) {
        StlAPI_Writer writer;
        writer.ASCIIMode() = m_params.format == Format::Ascii;
        return writer.Write(m_shape, filepath.u8string().c_str());
    }
    else if (!m_mesh.IsNull()) {
        Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
        const std::string filepathUtf8 = filepath.u8string();
        const OSD_Path osdFilepath(filepathUtf8.c_str());
        if (m_params.format == Format::Ascii)
            return RWStl::WriteAscii(m_mesh, osdFilepath, TKernelUtils::start(indicator));
        else
            return RWStl::WriteBinary(m_mesh, osdFilepath, TKernelUtils::start(indicator));
    }

    return false;
}

std::unique_ptr<PropertyGroup> OccStlWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccStlWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr)
        m_params.format = ptr->targetFormat;
}

} // namespace IO
} // namespace Mayo
