/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_stl.h"

#include "../base/application_item.h"
#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/triangulation_annex_data.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/global.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/occ_progress_indicator.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <RWStl.hxx>
#include <StlAPI_Writer.hxx>
#include <TDataStd_Name.hxx>
#include <TopoDS_Compound.hxx>

namespace Mayo::IO {

namespace {

static TopoDS_Shape asShape(const DocumentPtr& doc)
{
    TopoDS_Shape shape;

    if (doc->entityCount() == 1) {
        shape = XCaf::shape(doc->entityLabel(0));
    }
    else if (doc->entityCount() > 1) {
        TopoDS_Compound cmpd = BRepUtils::makeEmptyCompound();
        for (int i = 0; i < doc->entityCount(); ++i)
            BRepUtils::addShape(&cmpd, XCaf::shape(doc->entityLabel(i)));

        shape = cmpd;
    }

    return shape;
}

} // namespace

struct OccStlWriterI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccStlWriterI18N)
};

class OccStlWriter::Properties : public PropertyGroup {
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->targetFormat.mutableEnumeration().changeTrContext(OccStlWriterI18N::textIdContext());
    }

    void restoreDefaults() override {
        this->targetFormat.setValue(Format::Binary);
    }

    PropertyEnum<OccStlWriter::Format> targetFormat{ this, OccStlWriterI18N::textId("targetFormat") };
};

bool OccStlReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    m_baseFilename = filepath.stem();
    m_mesh = RWStl::ReadFile(filepath.u8string().c_str(), TKernelUtils::start(indicator));
    return !m_mesh.IsNull();
}

TDF_LabelSequence OccStlReader::transfer(DocumentPtr doc, TaskProgress* /*progress*/)
{
    if (m_mesh.IsNull())
        return {};

    const TDF_Label entityLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(entityLabel, BRepUtils::makeFace(m_mesh));
    TriangulationAnnexData::Set(entityLabel); // IMPORTANT: pure mesh part marker!
    TDataStd_Name::Set(entityLabel, filepathTo<TCollection_ExtendedString>(m_baseFilename));
    return CafUtils::makeLabelSequence({ entityLabel });
}

bool OccStlWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
    m_shape = BRepUtils::makeEmptyCompound();
    System::visitUniqueItems(appItems, [=](const ApplicationItem& appItem) {
        if (appItem.isDocument()) {
            BRepUtils::addShape(&m_shape, asShape(appItem.document()));
        }
        else if (appItem.isDocumentTreeNode()) {
            const TDF_Label label = appItem.documentTreeNode().label();
            if (XCaf::isShape(label))
                BRepUtils::addShape(&m_shape, XCaf::shape(label));
        }
    });

    return !m_shape.IsNull();
}

bool OccStlWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    if (!m_shape.IsNull()) {
        bool facesMeshed = true;
        BRepUtils::forEachSubFace(m_shape, [&](const TopoDS_Face& face) {
            TopLoc_Location loc;
            const auto& mesh = BRep_Tool::Triangulation(face, loc);
            if (mesh.IsNull())
                facesMeshed = false;
        });
        if (!facesMeshed) {
#if OCC_VERSION_HEX <= OCC_VERSION_CHECK(7, 3, 0)
            this->messenger()->emitError(OccStlWriterI18N::textIdTr("Not all BRep faces are meshed"));
            return false; // Continuing would crash
#else
            this->messenger()->emitWarning(OccStlWriterI18N::textIdTr("Not all BRep faces are meshed"));
#endif
        }

        StlAPI_Writer writer;
        writer.ASCIIMode() = m_params.format == Format::Ascii;
        const std::string strFilepath = filepath.u8string();
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        auto indicator = makeOccHandle<OccProgressIndicator>(progress);
        return writer.Write(m_shape, strFilepath.c_str(), TKernelUtils::start(indicator));
#else
        MAYO_UNUSED(progress);
        return writer.Write(m_shape, strFilepath.c_str());
#endif
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

} // namespace Mayo::IO
