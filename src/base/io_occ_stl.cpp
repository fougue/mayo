/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_stl.h"
#include "caf_utils.h"
#include "occ_progress_indicator.h"
#include "property_enumeration.h"
#include "scope_import.h"
#include "task_progress.h"
#include <fougtools/occtools/qt_utils.h>

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

struct StlWriterParameters : public PropertyGroup {
    StlWriterParameters(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup),
          targetFormat(this, MAYO_TEXT_ID("Mayo::IO::OccStlWriter", "targetFormat"), &enumFormat())
    {
        this->targetFormat.setValue(int(OccStlWriter::Format::Binary));
    }

    static const Enumeration& enumFormat() {
        static const Enumeration values = {
            { int(OccStlWriter::Format::Ascii), MAYO_TEXT_ID("Mayo::IO::OccStlWriter", "StlAscii") },
            { int(OccStlWriter::Format::Binary), MAYO_TEXT_ID("Mayo::IO::OccStlWriter", "StlBinary") }
        };
        return values;
    }

    PropertyEnumeration targetFormat;
};

} // namespace

bool OccStlReader::readFile(const QString& filepath, TaskProgress* progress)
{
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    m_baseFilename = QFileInfo(filepath).baseName();
    m_mesh = RWStl::ReadFile(OSD_Path(filepath.toLocal8Bit().constData()), indicator);
    return !m_mesh.IsNull();
}

bool OccStlReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (m_mesh.IsNull())
        return false;

    SingleScopeImport import(doc);
    TDataXtd_Triangulation::Set(import.entityLabel(), m_mesh);
    CafUtils::setLabelAttrStdName(import.entityLabel(), m_baseFilename);
    progress->setValue(100);
    return true;
}

bool OccStlWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
//    if (appItems.size() > 1)
//        return Result::error(tr("OpenCascade RWStl does not support multi-solids"));

    m_shape = {};
    m_mesh = {};
    if (!appItems.empty()) {
        const ApplicationItem& item = appItems.at(0);
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

bool OccStlWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    if (!m_shape.IsNull()) {
        StlAPI_Writer writer;
        writer.ASCIIMode() = m_targetFormat == Format::Ascii;
        return writer.Write(m_shape, filepath.toLocal8Bit().constData());
    }
    else if (!m_mesh.IsNull()) {
        Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
        const QByteArray filepathLocal8b = filepath.toLocal8Bit();
        const OSD_Path osdFilepath(filepathLocal8b.constData());
        if (m_targetFormat == Format::Ascii)
            return RWStl::WriteAscii(m_mesh, osdFilepath, indicator);
        else
            return RWStl::WriteBinary(m_mesh, osdFilepath, indicator);
    }

    return false;
}

std::unique_ptr<PropertyGroup> OccStlWriter::createParameters(PropertyGroup* parentGroup)
{
    return std::make_unique<StlWriterParameters>(parentGroup);
}

void OccStlWriter::applyParameters(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const StlWriterParameters*>(params);
    if (ptr)
        this->setTargetFormat(ptr->targetFormat.valueAs<OccStlWriter::Format>());
}

} // namespace IO
} // namespace Mayo
